# ideas/notes
- cairo for drawing stuff (text on images, etc)
- libcurl for web requests
- sqlite for database needs
- opencv to load images (overkill I know, but it works + is fast)
- dear imgui for ui (fast, but not 'normal' looking)

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




