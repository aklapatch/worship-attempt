#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
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
#include <FL/Fl_Color_Chooser.H>

static std::vector<Fl_Image*> im_vec;

// The editing interface will look a little like Google slides.
// With the list of slides on the left and a preview more in the center.
// Different songs can be selected with a dropdown choose menu

class EditSlide: public Fl_Group {
    public:
        Fl_Image *img = NULL;
        Fl_Font font = FL_HELVETICA;
        Fl_Button im_button, pres_button;

    EditSlide(int x, int y, int w, int h, const char *label = "edit slide", Fl_Image* in_img=NULL) :
        Fl_Group(x, y, w, h, label),
        im_button(x, y, w, h),
        pres_button(x, y, w, h, "Present") {
        end();

        int txt_x, txt_y, txt_w, txt_h;
        fl_font(pres_button.labelfont(), pres_button.labelsize());
        fl_text_extents("Present", txt_x, txt_y, txt_w, txt_h);
        pres_button.resize(x, y + im_button.h(), txt_w, txt_h);
        this->resize(x, y, im_button.w() + 50, im_button.h() + pres_button.h());
    }
};

struct Song {
    Fl_Font font = FL_HELVETICA;
    Fl_Fontsize font_size = 50;
    std::vector<std::string> lyrics = {};
    std::string background_name = "";
};

struct Set {
    std::vector<Song> songs = {};
};

class PresWin : public Fl_Window {
    public:
        Fl_Fontsize font_size = 50; 
        uchar r = 0, g = 0, b = 0;
        Fl_Font font = FL_HELVETICA;
        Fl_Image *cur_img = NULL, *next_img = NULL;
        std::string cur_txt = "", next_txt = "";
    
    PresWin(int x, int y, int w, int h, const char *label = NULL) : Fl_Window(x, y, w, h, label) {}

    void draw() {
        cur_img->draw(0, 0);

        fl_color(r, g, b);
        fl_font(font, font_size);
        // try to put the text at the center of the screen
        int tmp_w, tmp_h;
        fl_measure(cur_txt.c_str(), tmp_w, tmp_h);
        int txt_x = this->x_root() + this->w()/2 - (tmp_w/2), txt_y = this->y_root() + (this->h()/2) - (tmp_h/2);
        fl_draw(cur_txt.c_str(), txt_x, txt_y, tmp_w, tmp_h, FL_ALIGN_CENTER, NULL, 0);
    }
};

static PresWin pres_win(0, 0, 0, 0, "Pres window");

#define STR(x) #x

Fl_Window editor_window(720, 720, "Editor Window");
Fl_Button present(300, 210, 80, 40, "Present");
Fl_Input_Choice font_in(100, 100, 200, 20, "font number");
Fl_Int_Input font_size_in(300, 300, 50, 30, "font size");
Fl_Input text_in(300, 510, 200, 30, "text in");
Fl_Color_Chooser color_in(400, 50, 200, 100, "font color");
Fl_Scroll slide_edit_list(50, 200, 0, 0, "Slides");

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

#define TOCHAR(x) (uchar)((x)*255.0)
void show_full_win(Fl_Widget *w, void *)
{
    pres_win.r = TOCHAR(color_in.r());
    pres_win.g = TOCHAR(color_in.g());
    pres_win.b = TOCHAR(color_in.b());
    pres_win.cur_txt = text_in.value();
    pres_win.cur_img = im_vec[0];
    pres_win.font_size = atoi(font_size_in.value());
    pres_win.font = font_val_map[font_in.value()];
    pres_win.show();
}

int main(int argc, char *argv[]){
    editor_window.end();
    pres_win.fullscreen();
    im_vec.push_back(Fl_PNG_Image("./worship extreme main menu.png").copy());

    for (auto x: font_val_map){
        font_in.add(x.first.c_str());
    }
    font_in.value(STR(FL_HELVETICA));
    font_size_in.value("50");
    // set font color to rgb mode
    color_in.mode(0);
    color_in.rgb(1, 1, 1);
    text_in.value("test label");
    present.callback(show_full_win);
    Fl_Button im_button(0, 0, 160, 90), p_button(0, im_button.h(), 0, 0, "Present");
    int txt_x, txt_y, txt_w, txt_h;
    fl_font(p_button.labelfont(), p_button.labelsize());
    fl_text_extents("Present", txt_x, txt_y, txt_w, txt_h);
    p_button.resize(txt_w, im_button.h(), txt_w + 10, txt_h + 10);
    im_button.image(im_vec[0]->copy(im_button.w(), im_button.h()));
    slide_edit_list.add(im_button);
    slide_edit_list.add(p_button);
    slide_edit_list.size(im_button.w() + slide_edit_list.scrollbar_size(), im_button.h() + slide_edit_list.scrollbar_size() +  p_button.h());

    int w = slide_edit_list.w();
    int h = slide_edit_list.h();

    editor_window.show();
    slide_edit_list.scroll_to(0, 0);
    slide_edit_list.redraw();
    return Fl::run();
}