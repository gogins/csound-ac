/*
* C S O U N D
*
* L I C E N S E
*
* This software is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifdef _MSC_VER
#define NOMINMAX
#endif
#include "CppSound.hpp"
#include "ImageToScore.hpp"
#include "System.hpp"
#include <cmath>
#include <complex>
#include <set>
#include <functional>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace csound {

    ImageToScore2::ImageToScore2() {
    }

    ImageToScore2::~ImageToScore2() {
    }

    void ImageToScore2::setImageFilename(std::string image_filename_) {
        image_filename = image_filename_;
    }

    std::string ImageToScore2::getImageFilename() const {
        return image_filename;
    }

    void ImageToScore2::setMaximumVoiceCount(size_t maximum_voice_count_) {
        maximum_voice_count = maximum_voice_count_;
    }

    size_t ImageToScore2::getMaximumVoiceCount() const {
        return maximum_voice_count;
    }

    void ImageToScore2::gaussianBlur(double sigma_x_, double sigma_y_, int kernel_size_, int kernel_shape_) {
        do_blur = true;
        sigma_x = sigma_x_;
        sigma_y = sigma_y_;
        kernel_size = kernel_size_;
     }

    void ImageToScore2::condense(int row_count_) {
        do_condense = true;
        row_count = row_count_;
    }

    void ImageToScore2::contrast(double gain_, double bias_) {
        do_contrast = true;
        gain = gain_;
    }

    void ImageToScore2::dilate(int kernel_shape_, int kernel_size_, int iterations_) {
        do_dilate = true;
        kernel_size = kernel_size_;
        iterations = iterations_;
    }

    void ImageToScore2::erode(int kernel_shape_, int kernel_size_, int iterations_) {
        do_erode = true;
        kernel_size = kernel_size_;
        iterations = iterations_;
    }

    void ImageToScore2::sharpen(int kernel_size_, double sigma_x_, double sigma_y_, double alpha_,
        double beta_, double gamma_) {
        do_sharpen = true;
        sigma_x = sigma_x_;
        sigma_y = sigma_y_;
        alpha = alpha_;
        beta = beta_;
        gamma = gamma_;
    }

    void ImageToScore2::threshhold(double value_threshhold_) {
        do_threshhold = true;
        value_threshhold = value_threshhold_;
    }

    void ImageToScore2::processImage() {
        System::inform("ImageToScore2::processImage...\n");
        System::inform("image_filename: %s\n", image_filename.c_str());
        int width;
        int height;
        int channels;
        float* data = stbi_loadf(image_filename.c_str(), &width, &height, &channels, 0);
        if (data == nullptr) {
            System::error("Failed to open image!\n");
            return;
        }
        if (channels < 3) {
            System::error("Image file must have at least 3 channels!\n");
            return;
        }
        original_image.assign(data, width, height, 1, channels, true);
        stbi_image_free(data);
        processed_image = original_image.get_RGBtoHSV();
        System::inform("Read image: columns: %d rows: %d channels: %d\n", width, height, channels);
        // First we process the image, then we translate it.
        if(do_blur) {
            System::inform("Blur...\n");
            processed_image.blur(sigma_x, sigma_y);
        }
        if(do_sharpen) {
            System::inform("Sharpen...\n");
            processed_image.sharpen(alpha);
         }
        if(do_erode) {
            System::inform("Erode..\n");
            for (int i = 0; i < iterations; ++i) {
                processed_image.erode(kernel_size);
            }
        }
        if(do_dilate) {
            System::inform("Dilate...\n");
            for(int i = 0; i < iterations; ++i) {
                processed_image.dilate(kernel_size);
            }
        }
        if(do_contrast) {
            System::inform("Contrast...\n");
            processed_image.get_channel(2) * gain;
        }
        if(do_condense) {
            System::inform("Condense...\n");
            processed_image.resize(width, row_count);
        }
        System::inform("ImageToScore2::processImage.\n");
    }

    csound::Event ImageToScore2::pixel_to_event(int column, int row) const {
        // 3rd parameter is slice, 0 for this 2-d image.
        float h = processed_image(column, row, 0, 0);
        float s = processed_image(column, row, 0, 1);
        float v = processed_image(column, row, 0, 2);
        csound::Event event_;
        double status = 144.;
        event_.setStatus(status);
        double time_ = column;
        event_.setTime(time_);
        double instrument = h;
        event_.setInstrument(instrument);
        double key = row;
        event_.setKey(key);
        double velocity = v;
        event_.setVelocity(velocity);
        return event_;
    }

    void ImageToScore2::generateLocally() {
        System::inform("ImageToScore2::generateLocally...\n");
        // Processing the image before translating can reduce the number
        // of salient notes.
        processImage();
        // Translate a processed image to notes. The width dimension of the image
        // represents time, and the height dimension of the image represents pitch.
        // The basic idea is that, along each row of pixels, if the value goes above
        // a threshhold, a note is starting; if the value goes below that
        // threshhold, the note is ending. It might be that too many notes are
        // created at a particular time. Therefore, the notes are ordered by
        // salience and only the most salient are retained. First of course we
        // translate to HSV, which seems to be the best color model for our
        // purposes.
        //
        score.clear();
        auto note_salience_less = [](const csound::Event &a, const csound::Event &b) {
            if (a.getVelocity() < b.getVelocity()) {
                return true;
            } else {
                return false;
            }
        };
        // Calculate min and max for each channel.
        double h_min, h_max, s_min, s_max, v_min, v_max;
        // Extract channels
        cimg_library::CImg<float> H = processed_image.get_channel(0);
        cimg_library::CImg<float> S = processed_image.get_channel(1);
        cimg_library::CImg<float> V = processed_image.get_channel(2);
        // Compute min and max values
        h_min = H.min(), h_max = H.max();
        s_min = S.min(), s_max = S.max();
        v_min = V.min(), v_max = V.max();
        std::cout << "Columns (time steps):        " << processed_image.width() << std::endl;
        std::cout << "Rows (distinct pitches):     " << processed_image.height() << std::endl;
        std::cout << "Hue (instrument): Min =      " << h_min << ", Max = " << h_max << std::endl;
        std::cout << "Saturation (not used): Min = " << s_min << ", Max = " << s_max << std::endl;
        std::cout << "Value (loudness): Min =      " << v_min << ", Max = " << v_max << std::endl;
        std::vector<csound::Event> prior_notes;
        std::map<int, csound::Event> prior_notes_for_rows;
        std::vector<csound::Event> current_notes;
        std::map<int, csound::Event> current_notes_for_rows;
        std::map<int, csound::Event> pending_notes_for_rows;
        // Score time is simply column index. This should later be rescaaled.
        for (int prior_column = 0, current_column = 1; current_column < processed_image.width(); ++prior_column, ++current_column) {
            // Score pitch is simply row index. This should later be rescaled. 
            // pitch is upside down. Only one note may sound on a row at any 
            // given time.
            System::inform("Processing column %6d\n", prior_column);
            for (int row = processed_image.height() - 1; row >= 0; --row) {
                auto prior_note = pixel_to_event(prior_column, row);
                prior_notes.push_back(prior_note);
                auto current_note = pixel_to_event(current_column, row);
                current_notes.push_back(current_note);
            }
            // Sort the notes by decreasing loudness.
            std::sort(prior_notes.begin(), prior_notes.end(), note_salience_less);
            std::sort(current_notes.begin(), current_notes.end(), note_salience_less);
            // Remove all excess notes. Louder notes come first.
            if (maximum_voice_count < prior_notes.size()) {
                prior_notes.erase(prior_notes.begin() + maximum_voice_count, prior_notes.end());
            }
            if (maximum_voice_count < current_notes.size()) {
                current_notes.erase(current_notes.begin() + maximum_voice_count, current_notes.end());
            }
            // A note is beginning if a current note has no matching prior note. 
            // Add it to the pending notes.
            for (auto &current_note : current_notes) {
                auto current_key = current_note.getKey();
                if (prior_notes_for_rows.find(current_key) == prior_notes_for_rows.end()) {
                   pending_notes_for_rows[current_key] = current_note;
                }
            }
            // A note is ending if a prior note has no matching current note.
            // Set the pending note's off time and add it to the score.
            for (auto &prior_note : prior_notes) {
                auto prior_key = prior_note.getKey();
                if (current_notes_for_rows.find(prior_key) == current_notes_for_rows.end()) {
                    auto &pending_note = pending_notes_for_rows[prior_key];
                    pending_note.setDuration(current_column - pending_note.getTime());
                    if ((pending_note.getDuration() > 0) && (pending_note.getStatus() == 144)) {
                        System::inform("Adding note: %s\n", pending_note.toString().c_str());
                        score.push_back(pending_note);
                    }
                    pending_notes_for_rows.erase(pending_note.getKey());
                }
            }
            prior_notes.clear();
            prior_notes_for_rows.clear();
            current_notes.clear();
            current_notes_for_rows.clear();
        }
        System::inform("ImageToScore2::generateLocally with %d events.\n", score.size());
    }

}
