# Read-and-write-AVI-QT
This is a program that can read a raw video (RBG, YUV4:2:0 and YUYV) and AUDIO (PCM) AVI File and Wite raw video from webcam and audio from mic.
The Program doesnt use any Multi-media libaries (FFMPEG, Direct-show, etc) to read and write AVI.

I wanted to challenge myself to reverse engineer a raw AVI file. I like to challenge my self to wirte progrmes without using third party libaries and try to stick to native C and C++.

In this project, I tried to test diffrent methods of converting the image formats to find out the most efficient algorithms. I also tested diffrent compilers mainly MSVC vs MingW to see which compiles efficient code better.
