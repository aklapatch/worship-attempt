# ideas/notes
- libcurl for web requests  (MIT license)
- maybe I need to not load all the songs at once. I may need to load all the songs 
  more lazily through the database, but I can't think of a way to do that without
  repeatedly reading to the database every draw call(THAT'S REALLY BAD). 
  
## TODO: feature list
- Hot song editing/reloading
- revise ui into tabs
- Implement text shadowing, italics, and find a way to load in all the fonts that windows has installed
- text color, justification, and other stuff
- schedule saving/import
- background image selection
- overwrite images with duplicate names (or data)
- overwrite songs with duplicate names (warn FIRST!!!)
- Implement song ordering (use a hash map or something)
- image_menu button wrapping

## TODO: usability
- warnings ifa  song is not formatted correctly
- warnings if a song is being overwritten (name collision)
- building guide + makpkg script
- static linking:
  - build opencv statically (that's probably a nightmare)
  - build everyting that is not static statically

## song storage
Make a dir structure:
- sets folder -> sets of songs 
  - JSON or toml with song names and backgrounds
- songs -> song layouts (plain text files)
  - JSON or toml with song verses and linked backgrounds/images
- images -> song backgrounds (png/jpg)
This structure should originate at ~/ or appdata

## ui notes
- I still need to figure out which window placement is best. 
- I need:
  - search dialog (connected to list of all songs) (partially done)
  - preview window for the song (with a way to select the versese and display their text
  - song text editor, place to update the song and re-save it. (schedule does not update yet)
  - hot editing should affect only the schedule copy of the song, but the 'save' button will make it save to the databse.
  - saving the schedule will save the changes that you made to the schedule-songs, but to a separate file.