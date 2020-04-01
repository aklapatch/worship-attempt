# ideas/notes
- cairo for drawing stuff (text on images, etc)  (GNU LIbrary GPL)
- libcurl for web requests  (MIT license)
- sqlite for database needs (public domain?)
- opencv to load images (overkill I know, but it works + is fast) (BSD 3 clause license) 
- dear imgui for ui (fast, but not 'normal' looking) (MIT License)

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




