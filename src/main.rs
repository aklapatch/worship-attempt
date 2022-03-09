use fltk::{app, button::Button, frame::Frame, prelude::*, window::Window, image::*, enums };
use std::cell::RefCell;

#[derive(Debug, Clone, Copy)]
pub enum Message{ 
    Show,
    NextImg,
    PrevImg,

}

fn main() {
    let app = app::App::default().with_scheme(app::Scheme::Gleam);
    let mut wind = Window::new(100, 100, 400, 300, "Editor window");

    let mut frame = Frame::new(0, 0, 400, 200, "");
    let mut but = Button::new(160, 210, 80, 40, "open window");
    wind.make_resizable(true);
    wind.clear_modal_states();
    wind.end();
    wind.show();

    let mut wind1 = Window::new(0, 0, 400, 300, "display_window");
    wind1.clear_modal_states();
    wind1.fullscreen(true);

    let mut img_frame : Frame = Frame::default().size_of(&wind1);
    wind1.end();
    wind1.hide();

    let img_arr = vec![ SharedImage::load("./worship extreme main menu.png").expect("Failed"), SharedImage::load("./sunset.jpg").expect("failed") ];

    let (s, r) = app::channel::<Message>();

    but.set_callback( move |_| s.send(Message::Show));

    img_frame.handle( move |_, ev| match ev {
            enums::Event::KeyUp => {
            if app::event_key() == enums::Key::Right {
                s.send(Message::NextImg);
            } else if app::event_key() == enums::Key::Left {
                s.send(Message::PrevImg);
            }
            true
        }
        _ => false,
    });

    let mut img_i: usize = 0;
    while app.wait() {
        if let Some(msg) = r.recv() {
            match msg {
                Message::Show => {
                    wind1.show();
                    img_frame.set_size(wind1.pixel_w(), wind1.pixel_h());
                    img_frame.set_pos(wind1.x(), wind1.y());
                    img_frame.set_image_scaled(Some(img_arr[0].clone()));
                    img_frame.redraw();
                },
                Message::NextImg => {
                    if img_i < img_arr.len() - 1 {
                        img_i += 1;
                    }
                    img_frame.set_image_scaled(Some(img_arr[img_i].clone()));
                    wind1.redraw();
                },
                Message::PrevImg => {
                    if img_i > 0 {
                        img_i -= 1;
                    }
                    img_frame.set_image_scaled(Some(img_arr[img_i].copy()));
                    wind1.redraw();
                },
                _ => {}
            }
        }
    }
}

