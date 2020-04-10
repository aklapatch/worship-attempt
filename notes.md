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
- 

## database design (SQLITE)
- songs table
    - id 
    - title
    - author
    - copyright stuff
    - song assoc id 
    - background for song (name/id for image)

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
    - id

- sched-song-link
    - sched id 
    - song id's




