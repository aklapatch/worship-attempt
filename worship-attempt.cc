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

#define TXT_MARGIN (10)

class EditSlide: public Fl_Group {
    Fl_Button im_button, pres_button;
    public:
        Fl_Image *img = NULL;
        Fl_Font font = FL_HELVETICA;
        std::string lyric = "";

    EditSlide(int x, int y, int w, const char *label = "edit slide", Fl_Image* in_img=NULL) :
        // The 1 here needs to stay a 1. Tried 0 and 
        // The image button didn't come up
        Fl_Group(x, y, w, 1, label),
        im_button(x, y, w, 0),
        pres_button(x, y, w, 0, "Present") {
        end();

        int txt_x, txt_y, txt_w, txt_h;
        fl_font(pres_button.labelfont(), pres_button.labelsize());
        fl_text_extents("Present", txt_x, txt_y, txt_w, txt_h);
        txt_w += TXT_MARGIN, txt_h += TXT_MARGIN;
        // keep the w over txt_w
        w = txt_w > w ? txt_w : w;

        //adjust the image button to be a 16/9 aspect ratio
        int ratio_h = (w*9)/16;
        // doing this resize after resizing the widgets messes things up.
        this->size(w, ratio_h + txt_h);

        im_button.size(w, ratio_h);
        pres_button.resize(x, y + ratio_h, w, txt_h);
    }
};

// current list of songs that can be edited
static struct {
    Fl_Font font = FL_HELVETICA;
    Fl_Fontsize font_size = 50;
    std::vector<std::string> lyrics = {};
    std::string background_name = "";
} song_edit_list;

struct SlideAndButton {
    // full size image. The Fl_Button takes a copy
    Fl_Image *in_img = NULL;
    Fl_Button im, pres;
    Fl_Font font = FL_HELVETICA;
    Fl_Fontsize f_size = 50;
    std::string lyrics = "";
};

struct Slides {
    Fl_Font font = FL_HELVETICA;
    Fl_Fontsize font_size = 50;
    std::vector<std::string> lyrics = {};
    Fl_Image *full_size_im = NULL;
    std::string background_name = "";
};

static Slides edit_slides;

struct Set {
    std::vector<Slides> slides = {};
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
Fl_Button add_slide_b(200, 150, 25, 25, "+");
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
    EditSlide slide(slide_edit_list.x(), slide_edit_list.y(), 100);
    slide_edit_list.add(slide);
    slide_edit_list.size(slide.w() + slide_edit_list.scrollbar_size(), slide.h() + slide_edit_list.scrollbar_size());

    int w = slide_edit_list.w();
    int h = slide_edit_list.h();

    editor_window.show();
    slide_edit_list.scroll_to(0, 0);
    slide_edit_list.redraw();
    return Fl::run();
}