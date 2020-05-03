#include "imageMenu.h"

// keep a list of images in the database (selectables that preview when clicked)
//
bool imageMenu(std::vector<image> &image_list) {
ImGui::Begin("image menu");
    
    static bool open_file_menu = false;
    static std::string new_img = "";


        ImGui::Text("image list");
        ImGui::SameLine();
        if(ImGui::Button("add image")){

            open_file_menu = true;
        }
        ImGui::Separator();
        int32_t i = -1;
        static int32_t selected = -1;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("img list", ImVec2(ImGui::GetWindowContentRegionWidth()*0.5f, 0), true, window_flags);
        if (image_list.size() == 0){
            ImGui::Text("No images were found!");
        }
        for (auto x: image_list){
            i+=1;

            if(ImGui::Selectable(x.name.c_str(), i == selected)){
                    selected = i;
                    }
                    }

        ImGui::EndChild();



        ImGui::End();

        // open the file menu here
        if (open_file_menu){
            open_file_menu = !(fileMenu(new_img));
            // if the string size is > 0, then there is a file to load.
            if (new_img.size() > 0){
                // load the image
                // open the filel with opencv
                std::cerr << new_img << "\n";
          cv::Mat orig_image = cv::imread(new_img.c_str(), cv::IMREAD_COLOR);
          auto img_arr = cv::InputArray(orig_image);
          // use jpg compression (it's fast)
          std::vector<unsigned char> img_out = {};
          // lossless jpg compression
            bool rc = cv::imencode(".jpg", img_arr, img_out, {cv::IMWRITE_JPEG_QUALITY, 100});

            // pass the image buffer and the filename to be saved in the database.
            // adding the +1 excludes the '/'
            size_t last_slash_i =  new_img.rfind("/") + 1; 
            std::string img_name = new_img.substr(last_slash_i, new_img.size() - last_slash_i);
                std::cerr << img_name<< "\n";
                saveImg(img_out, img_name);

            open_file_menu = false;
            }
        }


    return false;

}

