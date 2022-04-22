#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Native_File_Chooser.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Pixmap.H>
#include <FL/platform.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Int_Input.H>

static std::vector<Fl_Image*> im_vec;

class PresWin : public Fl_Window {
    public:
        Fl_Fontsize font_size = 50; 
        Fl_Font font = FL_HELVETICA;
        int x = 0, y = 0, w = 200, h = 200;
        Fl_Color text_color = FL_WHITE;
        Fl_Image *cur_img = NULL, *next_img = NULL;
        std::string cur_txt = "", next_txt = "";
    
    PresWin(int x, int y, int w, int h, const char *label = NULL) : Fl_Window(x, y, w, h, label) {}

    void draw() {
        cur_img->draw(0, 0);
        fl_color(text_color);
        fl_font(font, font_size);
        fl_draw(cur_txt.c_str(), x, y, w, h, FL_ALIGN_CENTER, NULL, 0);
    }
};

static PresWin pres_win(0, 0, 0, 0, "Pres window");

#define STR(x) #x

Fl_Window editor_window(720, 720, "Editor Window");
Fl_Button present(160, 210, 80, 40, "Present");
Fl_Input_Choice font_in(100, 100, 200, 20, "font number");
Fl_Int_Input font_size_in(300, 300, 50, 30, "font size");
Fl_Int_Input x_in(300, 370, 50, 30, "x");
Fl_Int_Input y_in(300, 440, 50, 30, "y");
Fl_Input text_in(300, 510, 200, 30, "text in");

#define MAP_FONT(x) STR(x), x
std::map<std::string, Fl_Font> font_val_map = {
    {MAP_FONT(FL_HELVETICA)},
    {MAP_FONT(FL_HELVETICA_BOLD)},
    {MAP_FONT(FL_HELVETICA_ITALIC)},
    {MAP_FONT(FL_COURIER)},
    {MAP_FONT(FL_COURIER_BOLD)},
    {MAP_FONT(FL_COURIER_ITALIC)},
    {MAP_FONT(FL_COURIER_BOLD_ITALIC)},
    {MAP_FONT(FL_TIMES)},
    {MAP_FONT(FL_TIMES_BOLD)},
    {MAP_FONT(FL_TIMES_ITALIC)},
    {MAP_FONT(FL_TIMES_BOLD_ITALIC)},
};

void show_full_win(Fl_Widget *w, void *)
{
    pres_win.cur_txt = text_in.value();
    pres_win.cur_img = im_vec[0];
    pres_win.x = atoi(x_in.value());
    pres_win.y = atoi(y_in.value());
    pres_win.font_size = atoi(font_size_in.value());
    pres_win.font = font_val_map[font_in.value()];
    pres_win.show();
}

int main(int argc, char *argv[]){
    pres_win.fullscreen();
    im_vec.push_back(Fl_PNG_Image("./worship extreme main menu.png").copy());

    for (auto x: font_val_map){
        font_in.add(x.first.c_str());
    }
    font_in.value(STR(FL_HELVETICA));
    x_in.value("700");
    y_in.value("700");
    font_size_in.value("50");
    text_in.value("test label");
    present.callback(show_full_win);

    editor_window.show();
    return Fl::run();
}