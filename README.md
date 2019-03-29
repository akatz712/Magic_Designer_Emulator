# Magic_Designer_Emulator
This computer program is an exact emulation of a toy which was sold years ago called Magic Designer . It also once had the name Hoot Nanny. It is made out of metal and comes with circular paper and colored pens.

Fun artistic tool set for producing mathematically inspired designs.

Allows you to produce an almost infinite number of spectacular designs. Simply enjoy, or use Magic Designs for rad T-shirt transfers, cool CD/DVD labels, vivid transparencies, glam greeting cards, boffo biz logos, or even as patterns for unique tattoos.

The emulator is based on a 1929 invention by Howard B. Jones of Chicago, Illinois. Originally called Hoot Nanny, the Magic Designer was marketed for about 50 years by the Northern Signal Co. of Saukville, Wisconsin. Its 6 inch diameter Center Gear meshed with two 1-inch Planetary Gears, each with a circling Peg. The Pen Arms were placed on these Gear Pegs or on fixed Stud Pegs at either side. The Left Gear was moved in a 60 degree arc by the Shift Lever. By selecting holes in the arms and a setting on the shift lever, brilliant designs could be cranked out on rotating paper discs.

The program in GitHub is based on the Google Play Android app by the same name. 

It uses The High Fidelity 2D Graphics Rendering Engine for C++ known as Anti-Grain Geometry (AGG). It is an Open Source, free of charge graphic library, written in industrially standard C++. AGG doesn't depend on any graphic API or technology. Basically, you can think of AGG as of a rendering engine that produces pixel images in memory from some vectorial data. For more information, see http://www.antigrain.com/about/index.html This was chosen because it offers anti-aliasing. And, because I found an easier to use modification of it here - https://sourceforge.net/p/agg/svn/HEAD/tree/agg-2.4/ captured on January 24,2019 known as r132.

I have chosen only the source from the AGG library that I actually needed, and static linked it to the source which I wrote, so it is included with the GitHub repository for this program.

It uses FLTK as its GUI toolkit. For Linux, I am assuming that the development package has been installed. https://www.fltk.org/ The program works with release 1.3.5 and it is anticipated to work with 1.3.x or future releases. If any changes to FLTK cause problems, those will be fixed.

For PDF output, the program uses http://libharu.org/ For Linux, I am assuming that the development package has been installed. The current release is 2.3.0.

The emulator is based upon solving mathematical formulas to calculate the point at which the arms meet given all of the current settings and an incremental turning of the Center gear. These series of points are connected in a smooth curve. And this constitutes one design. Using the emulator to save many designs together is what makes the result so beautiful.

There are generally two types of design suites one would make. One would be a series of closely packed varied designs. This is known as a morph. You make the first and last design, and tell the emulator to calculate and draw the intermediate designs. The second type is drawing a few designs and painting (coloring in) the overlapping symmetrical shapes. One can save the results to either PDF (Adobe scalable form) file or make a screen capture with an external tool. One can also load and save custom text design specification files (MGS). The files can be shared with either other users of the app, or with other implementations of the emulator.

My website http://www.akatz712.com/ explains in detail the mechanics of the magic designer device, the mathematics behind it, and coding details. So, this program can be considered to have both educational and artistic value.

The emulator is an extension of the capabilities of the original device. The physical crank sometimes interferes with some drawings. This does not happen on the computer. The clipping is turned off by default, thus drawing off the paper. A design can be discontinuous, meaning that parts of it are impossible to draw. This can be done with the device, but it is very difficult and can damage it. Placement of designs, which is like having multiple papers on one screen. Mathematical alterations such as zooming and warping. More color choices. Various changes in configurations. For example, values for arms and shift which are either in between or out of range of the device. Changing the gear sizes, the distance and angles of the pegs. And do not forget the ability to erase mistakes is easier on a computer.

There are two other documents - BUILDME and USEME - which descibe how to compile the software and how to use the program.

The Windows version is released as a single exe file.

There is a separate document for the License, but to summarize, the software is being offered for free and can be used or modified anyway you want. The agg portion has its own similar license file.
