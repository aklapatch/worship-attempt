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
            if (new_img.size() > 0){
                // load the image
                //saveImg(new_img);

            }
        }


    return false;

}

