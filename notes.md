# ideas/notes
- cairo for drawing stuff (text on images, etc)  (GNU LIbrary GPL)
- libcurl for web requests  (MIT license)
- sqlite for database needs (public domain?)
- opencv to load images (overkill I know, but it works + is fast) (BSD 3 clause license) 
- dear imgui for ui (fast, but not 'normal' looking) (MIT License)
- maybe I need to not load all the songs at once. I may need to load all the songs 
  more lazily through the database, but I can't think of a way to do that without
  repeatedly reading to the database every draw call(THAT'S REALLY BAD). 

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
- I almost have song editing working, but the probelm is that when I try to save a song in the schedule, then the schedule songe does not update, and the rendered song selection freaks out. 
	
## TODO: usability
- warnings ifa  song is not formatted correctly
- warnings if a song is being overwritten (name collision)

## ui notes
- I still need to figure out which window placement is best. 
- I need:
  - list of all songs
  - lsit of current shedule songs
  - search dialog (connected to list of all songs)
  - preview window for the song (with a way to select the versese and display their text
  - song text editor, place to update the song and re-save it.
  - that hot editing should affect only the schedule copy of the song, but the 'save' button will make it save to the databse.
  - saving the schedule will save the changes that you made to the schedule-songs, but to a separate file.

## database design (SQLITE)
- songs table
    - id 
    - title
    - author
    - copyright stuff
    - song assoc id 
    - background for song (name/id for image)
    - font name
    - font size 
    - font shadowing settings

- song part assoc table
   - song_id
   - part_id

- song part table
    - id 
    - part_type(tag, verse, chorus,)
    - contents

- images
    - name
    - id
    - data (png, jpg data)

- schedule (for songs)
  	- font_size?
	- font_style? (bold, semi-bold, shadow, etc) (enums probably)
    - id

- sched-song-link
    - sched id 
    - song id's




