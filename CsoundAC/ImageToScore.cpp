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
#include <opencv2/imgcodecs.hpp>

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
        kernel_shape = kernel_shape_;
    }

    void ImageToScore2::condense(int row_count_) {
        do_condense = true;
        row_count = row_count_;
    }

    void ImageToScore2::contrast(double gain_, double bias_) {
        do_contrast = true;
        gain = gain_;
        bias = bias_;
    }

    void ImageToScore2::dilate(int kernel_shape_, int kernel_size_, int iterations_) {
        do_dilate = true;
        kernel_size = kernel_size_;
        kernel_shape = kernel_shape_;
        iterations = iterations_;
    }

    void ImageToScore2::erode(int kernel_shape_, int kernel_size_, int iterations_) {
        do_erode = true;
        kernel_size = kernel_size_;
        kernel_shape = kernel_shape_;
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

    void ImageToScore2::write_processed_file(std::string operation, const cv::Mat &processed_image) const {
        char buffer[0x200];
        std::snprintf(buffer, 0x200, "%s-%s.jpg", image_filename.c_str(), operation.c_str());
        cv::imwrite(buffer, processed_image);
    }

    void ImageToScore2::processImage() {
        System::inform("ImageToScore2::processImage...\n");
        System::inform("image_filename: %s\n", image_filename.c_str());
        original_image = cv::imread(image_filename.c_str(), cv::IMREAD_UNCHANGED);
        if (original_image.empty()) {
            System::error("Failed to open image!\n");
            return;
        }
        // Ensure the image has 3 channels (convert grayscale to RGB if needed)
        cv::Mat imageBGR;
        if (original_image.channels() == 1) {
            cv::cvtColor(original_image, imageBGR, cv::COLOR_GRAY2BGR);
        } else {
            imageBGR = original_image; // Already in a compatible format
        }
        cv::Mat imageHSV;
        cv::cvtColor(imageBGR, imageHSV, cv::COLOR_BGR2HSV);
        cv::Mat imageHSVFloat;
        imageHSV.convertTo(imageHSVFloat, CV_32FC3);
        auto element_size = imageHSV.elemSize();    
        std::cout << "Original image type:  " << cv::typeToString(original_image.type()) << std::endl;
        std::cout << "       Element size:  " << original_image.elemSize() << std::endl;
        std::cout << "Converted image type: " << cv::typeToString(imageHSVFloat.type())<< std::endl;
        std::cout << "        Element size: " << imageHSVFloat.elemSize() << std::endl;
        System::inform("Loaded image file \"%s\".\n", image_filename.c_str());
        System::inform("Read image: columns: %d rows: %d type: %d depth: %d\n", original_image.cols, original_image.rows, original_image.type(), original_image.depth());
        // First we process the image, then we translate it.
        cv::Mat source_image = imageHSVFloat;
        if(do_blur) {
            System::inform("Blur...\n");
            cv::Mat output_image;
            cv::GaussianBlur(source_image, output_image, cv::Size(kernel_size, kernel_size), sigma_x, sigma_y);
            write_processed_file("Blur", output_image);
            source_image = output_image;
        }
        if(do_sharpen) {
            System::inform("Sharpen...\n");
            cv::Mat output_image;
            cv::GaussianBlur(source_image, output_image, cv::Size(kernel_size, kernel_size), sigma_x, sigma_y);
            cv::Mat sharpened_image;
            cv::addWeighted(source_image, alpha, output_image, beta, gamma, sharpened_image);
            write_processed_file("Sharpen", output_image);
            source_image = sharpened_image;
        }
        if(do_erode) {
            System::inform("Erode..\n");
            cv::Mat output_image;
            cv::Mat kernel = cv::getStructuringElement(kernel_shape, cv::Size(kernel_size, kernel_size));
            for(int i = 0; i < iterations; ++i) {
                cv::erode(source_image, output_image, kernel);
                source_image = output_image;
            }
            write_processed_file("Erode", output_image);
        }
        if(do_dilate) {
            System::inform("Dilate...\n");
            cv::Mat output_image;
            cv::Mat kernel = cv::getStructuringElement(kernel_shape, cv::Size(kernel_size, kernel_size));
            for(int i = 0; i < iterations; ++i) {
                cv::dilate(source_image, output_image, kernel);
                source_image = output_image;
            }
            write_processed_file("Dilate", output_image);
        }
        if(do_contrast) {
            System::inform("Contrast...\n");
            cv::Mat output_image =  cv::Mat::zeros(source_image.size(), source_image.type());
            source_image.convertTo(output_image, -1, gain, bias);
            write_processed_file("Contrast", output_image);
            source_image = output_image;
        }
       if(do_condense) {
            System::inform("Condense...\n");
            cv::Mat output_image;
            cv::resize(source_image, output_image, cv::Size(source_image.cols, row_count), cv::INTER_AREA);
            System::inform("New size columns: %5d  rows: %5d\n", output_image.cols, output_image.rows);
            write_processed_file("Condense", output_image);
            source_image = output_image;
        }
        processed_image = source_image;
        write_processed_file("Processed", processed_image);
        System::inform("ImageToScore2::processImage.\n");
    }

    csound::Event ImageToScore2::pixel_to_event(int column, int row) const {
        const cv::Vec3f &hsv = processed_image.at<cv::Vec3f>(row, column);
        csound::Event event_;
        double status = 144.;
        event_.setStatus(status);
        double time_ = column;
        event_.setTime(time_);
        double instrument = hsv[0];
        event_.setInstrument(instrument);
        double key = row;
        event_.setKey(key);
        double velocity = hsv[2];
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
        // First we print out the size of the image on all dimensions.
        // Split the HSV image into H, S, and V channels
        std::vector<cv::Mat> channels;
        cv::split(processed_image, channels);
        // Calculate min and max for each channel.
        double h_min, h_max, s_min, s_max, v_min, v_max;
        cv::minMaxLoc(channels[0], &h_min, &h_max); // H channel
        cv::minMaxLoc(channels[1], &s_min, &s_max); // S channel
        cv::minMaxLoc(channels[2], &v_min, &v_max); // V channel
        std::cout << "Columns (time steps):        " << processed_image.cols << std::endl;
        std::cout << "Rows (distinct pitches):     " << processed_image.rows << std::endl;
        std::cout << "Hue (instrument): Min =      " << h_min << ", Max = " << h_max << std::endl;
        std::cout << "Saturation (not used): Min = " << s_min << ", Max = " << s_max << std::endl;
        std::cout << "Value (loudness): Min =      " << v_min << ", Max = " << v_max << std::endl;
        std::vector<csound::Event> prior_notes;
        std::map<int, csound::Event> prior_notes_for_rows;
        std::vector<csound::Event> current_notes;
        std::map<int, csound::Event> current_notes_for_rows;
        std::map<int, csound::Event> pending_notes_for_rows;
        // Score time is simply column index. This should later be rescaaled.
        for (int prior_column = 0, current_column = 1; current_column < processed_image.cols; ++prior_column, ++current_column) {
            // Score pitch is simply row index. This should later be rescaled. 
            // In OpenCV, {0, 0} is the center of the upper left pixel, so 
            // pitch is upside down. Only one note may sound on a row at any 
            // given time.
            System::inform("Processing column %6d\n", prior_column);
            for (int row = processed_image.rows - 1; row >= 0; --row) {
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
