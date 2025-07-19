#ifndef CSOUND_PRODUCER_HPP_INCLUDED
#define CSOUND_PRODUCER_HPP_INCLUDED

#include <csound_threaded.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace csound {
            
    /**
     * Shells out to run playpen.py to post-process the output soundfile. 
     * Metadata tags passed here are not used, metadata now comes from 
     * playpen.ini and playpen.py. Python version 3 must be in the executable 
     * path, and the playpen.py and playpen.ini files must be in the user's 
     * home directory.
     */
    static void PostProcess(std::map<std::string, std::string> &tags, std::string filename, CsoundThreaded *csound) {
        if (csound == nullptr) {
            std::fprintf(stderr, "Began PostProcess(%s)...\n", filename.c_str());
        } else {
            csound->Message("Began PostProcess(%s)...\n", filename.c_str());
        }
        char buffer[0x1000];
        const char *command = "python3 ~/playpen.py post-process %s\n";
        std::snprintf(buffer, 0x1000, command, filename.c_str());
        std::system(buffer);
        std::fprintf(stderr, "Ended PostProcess.\n");
    }
    
    /** 
     * Optionally adds metadata, performs post-processing, translates to 
     * various soundfile formats as automatic steps in the Csound rendering of 
     * a composition to a soundfile. Also enables running scripts that 
     * can interact with Csound.
     */
    class CsoundProducer : public CsoundThreaded {
        public:
            CsoundProducer() {
                // Inject this class' Csound object into the runtime context 
                // of various scripting languages.
            }
            virtual ~CsoundProducer() {
            }
            virtual clock_t startTiming() {
                return clock();
            }
            virtual double stopTiming(clock_t beganAt)
            {
                clock_t endedAt = clock();
                clock_t elapsed = endedAt - beganAt;
                return double(elapsed) / double(CLOCKS_PER_SEC);
            }
            /**
             * If enabled, assumes that the code embedding this piece is within a 
             * Git repository, and commits the repository before rendering the piece 
             * to ensure a consistent history of revisions of the piece.
             */
            virtual void GitCommit() {
                if (do_git_commit == true) {
                    auto result = std::system("git commit -a -m \"Automatically committed before rendering.\"");
                }
            }
            /**
             * If Git commit is enabled, assumes that the code embedding this 
             * piece is within a Git repository and returns the current Git hash 
             * of HEAD, to facilitate ensuring a consistent history of revisions 
             * of the piece. Otherwise, returns an empty string.
             */
            virtual std::string GetGitCommitHash() {
                if (do_git_commit == true) {
                    char temporary_filename[0x200];
                    auto discard = std::tmpnam(temporary_filename);
                    char command[0x300];
                    std::snprintf(command, sizeof(command), "git rev-parse --short HEAD > %s", temporary_filename);
                    auto result = std::system(command);
                    std::fstream hash_file(temporary_filename);
                    char hash[0x100];
                    hash_file.getline(hash, 0x200);
                    return hash;
                } else {
                    return "";
                }
            }
            virtual std::string GetFilenameBase() {
                std::string filename_base = GetMetadata("title");
                if (do_git_commit == true) {
                    filename_base.append("-");
                    filename_base.append(GetGitCommitHash());
                }
                return filename_base;
            }
            /**
             * Sets the value of a metadata tag. See:
             * https://www.ffmpeg.org/doxygen/trunk/group__metadata__api.html
             * Other and even user-defined tags may also be used.
             */
            virtual void SetMetadata(std::string tag, std::string value) {
                tags[tag] = value;
            }
            /** 
             * Returns the value of the metadata for the tag, or an empty 
             * string if the tag does not exist.
             */
            virtual std::string GetMetadata(std::string tag) const {
                if (tags.find(tag) != tags.end()) {
                    return tags.at(tag);
                } else {
                    return "";
                }
            }
            /**
             * Override to not only set but also save type and format.
             */
            virtual void SetOutput(const char *name, const char *type, const char *format) {
                output_type = type;
                output_format = format;
                Csound::SetOutput(name, type, format);
            }
            /**
             * Override to set output filename from metadata in this. Not 
             * implemented for real-time rendering.
             */        
            virtual int Start() {
                std::string output = GetOutputName();
                if (output.find("dac") != 0) {
                    output = GetFilenameBase();
                    output.append(".");
                    output.append(output_type);
                    Csound::SetOutput(output.c_str(), output_type.c_str(), output_format.c_str());
                }
                return CsoundThreaded::Start();
            }
            virtual int PerformAndPostProcessRoutine() {
                // We do our own performance benchmarks.
                auto clock_started = startTiming();
                Message("Began CsoundProducer::PerformAndPostProcessRoutine...\n");
                keep_running = true;
                int result = 0;
                while (true) {
                    if (keep_running == false) {
                        break;
                    }
                    CsoundEvent *event = 0;
                    while (input_queue.try_pop(event)) {
                        (*event)(csound);
                        delete event;
                    }
                    if (kperiod_callback != nullptr) {
                        kperiod_callback(csound, kperiod_callback_user_data);
                    }
                    result = Csound::PerformKsmps();
                    if (result != 0) {
                        Message("CsoundThreaded::PerformAndPostProcessRoutine: CsoundThreaded::PerformKsmps ended with %d...\n", result);
                        break;
                    }
                }
                keep_running = false;
                ClearQueue();
                Message("CsoundThreaded::PerformAndPostProcessRoutine: Cleared performance queue...\n");
                result = Cleanup();
                Message("CsoundThreaded::PerformAndPostProcessRoutine: CsoundThreaded::Cleanup returned %d...\n", result);
                auto seconds = stopTiming(clock_started);
                Message("Rendering took %9.4f seconds.\n", seconds);
                clock_started = startTiming();
                auto output_filename = GetFilenameBase();
                output_filename.append(".");
                output_filename.append(output_type);
                Message("Post-processing output file: %s.\n", output_filename.c_str());
                ///Message("Post-processing in CsoundProducer is deprecated, and is not performed.");
                PostProcess(tags, output_filename, this);
                seconds = stopTiming(clock_started);
                Message("Post-processing %s took %9.4f seconds.\n", output_filename.c_str(), seconds);                
                Message("Ended CsoundProducer::PerformAndPostProcessRoutine with %d.\n", result);
                Reset();
                Message("CsoundThreaded::PerformAndPostProcessRoutine: CsoundThreaded::Reset returned...\n");
                return result;
            }
            /**
             * Like PerformAndReset, but performs post-processing, translation, 
             * and tagging after rendering, so that these things are all done 
             * in the rendering thread.
             */
            virtual int PerformAndPostProcess() {
                performance_thread = std::thread(&CsoundProducer::PerformAndPostProcessRoutine, this);
                return 0;
            }
            virtual void SetDoGitCommit(bool do_git_commit_) {
                do_git_commit = do_git_commit_; 
            }
            virtual bool GetDoGitCommit() const {
                return do_git_commit;
            }
            /**
             * Causes the calling thread to wait for the end of the performance
             * thread routine.
             */
            virtual void Join()
            {
                Message("CsoundProducer::Join...\n");
                if (performance_thread.get_id() != std::this_thread::get_id()) {
                    if (performance_thread.joinable()) {
                        performance_thread.join();
                    }
                }
                Message("CsoundProducer::Join.\n");
            }
        protected:
            bool do_git_commit = false;
            std::map<std::string, std::string> tags;        
            std::string git_hash;
            std::string output_type = "wav";
            std::string output_format = "float";
    };
    
};

#endif
