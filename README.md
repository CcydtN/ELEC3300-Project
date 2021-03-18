# Music Player

It is a project that play music on stm32f103ve development board.

​	In **Music Player** folder, there are the source code for the Project.

​	In **Resource** folder, there are some link to resource we use and some reference.

## What it do

- Play music that store in a SD card with DAC w/ DMA

- A fully functional file browser

  - Display on a LCD screen
  - control by button connected with GPIO

- Volume control with a potential meter connected to ADC

- A Player controller

  - Display on a LCD screen

  - control by touch screen


### File System supported

- FAT/exFAT

### File Format Supported

- Wave (PCM)(8/16 bit)(Mono/Stereo)
- Ogg Vorbis* (Mono/Stereo)

> *Only some file can be played (not solved yet)
>
> maybe due to the memory usage cause only file with low compression rate can be play

### Open source Library Used

- tremor (low-memory branch) - https://git.xiph.org/?p=tremor.git

### Photo of Final Product

<img src="https://github.com/pynnek/ELEC3300_DEPRECATED/raw/master/others/photo1.jpg" />
