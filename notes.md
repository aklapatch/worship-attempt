# ideas/notes
- libcurl for web requests  (MIT license)
- maybe I need to not load all the songs at once. I may need to load all the songs 
  more lazily through the database, but I can't think of a way to do that without
  repeatedly reading to the database every draw call(THAT'S REALLY BAD). 
  
## Libs required to build
### MSYS2
All of these are the mingw-w64-x86_64 variants of these libraries.
- opencv
- sqlite 
- cairomm
- glfw
- More that I may have forgetten
### other
- imgui

## misc
- there was a memory leak in the image menu, I needed to implement a system where new textures are only generated/allocated if the image_list size changes

## TODO: What to work on next
- work on taking over a screen with glfw and displaying an image with text on the full screen.
  - bind ESC so that it stops taking over the screen when released
  - resize the background image to match dest screen (and then draw text on it
- Implement text shadowing, italics, and find a way to load in all the fonts that windows has installed
  -text color, justification, and other stuff
- schedule saving/import
- background image selection
- overwrite images with duplicate names (or data)
- overwrite songs with duplicate names (warn FIRST!!!)
- Implement song ordering (use a hash map or something)
- image_menu button wrapping

## TODO: optimizations (speed!!!!!)
- when redrawing the image, only redraw if they text has changed.
- make the shed_songs vector contain a song* instead of a song copy. 
  that should alleviate a lot of copies
- consolidate sql queries (into one query where possible)
- get size of song table to preallocate all_songs vector (may not be faster, since the extra file I/O may be slower than using push_back a lot.

## TODO: bugs 
- the menu where you move songs around glitches out and some songs move on their own sometimes.
- songs in the schedule section, if there are duplicates, selecting the duplicate fails to have the song titles show up in the selection dialog boxes
 - RELATED ^^^ : I cannot select duplicates in the schedule songs dialog (not sure why);
 - when you save a song, the schedule song list does not update with the new song (this is expected, since we copy those songs
      - however, the bug regarding selecting duplicates still exists

## TODO: usability
- warnings ifa  song is not formatted correctly
- warnings if a song is being overwritten (name collision)
- building guide + makpkg script
- static linking:
    - build opencv statically (that's probably a nightmare)
    - build everyting that is not static statically
    

## ui notes
- I still need to figure out which window placement is best. 
- I need:
  - search dialog (connected to list of all songs) (partially done)
  - preview window for the song (with a way to select the versese and display their text
  - song text editor, place to update the song and re-save it. (schedule does not update yet)
  - hot editing should affect only the schedule copy of the song, but the 'save' button will make it save to the databse.
  - saving the schedule will save the changes that you made to the schedule-songs, but to a separate file.


## database design (SQLITE)
SQLITE has built in rowid's so I don't need to include id's
- songs table
    - title
    - author
    - copyright stuff
    - song assoc id 
    - background for song (name/id for image)
    - font name
    - font size 
    - font shadowing settings

- images
    - name (include png,jpg suffix)
    - data (png, jpg data)

- schedule (for songs)
    - font_size?
    - font_style? (bold, semi-bold, shadow, etc) (enums probably)

- sched-song-link
    - song names (rowid + name)
    - song id's

