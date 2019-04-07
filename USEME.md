# **Magic Designer Overview**

This program is an emulation of a mechanical toy sold under various names including hoot-nanny and magic designer, in the 1960s and before. Here is a YouTube video showing this toy:
(https://www.youtube.com/watch?v=tdDhXOLNLc8)

It consists of interlocked gears and a V shaped pencil holder with holes to put on pegs. The ONLY difference the emulation makes, is that rather than turning a crank and rotating the main drawing surface, the emulation in effect has everything rotating around a fixed in place drawing surface. The final result is identical. The emulation works by calculating where the pencil meets the paper, taking samples 120 or more times. Bezier curves connected end to end do a great job of estimating the ever changing curvature.

The basic example is to draw one design, which is a closed curve of one full turn of 360 degrees of the central drawing surface. In the emulation, this happens instantly. And by specifying the parameters of the design, you can change it immediately. To add this design to the design suite, press the ADD button.

The only parameters which are in the toy are the shift lever and arms. The lever has integers 10 through 70, and the arms each have holes A through R. That is 61x18x18 possible designs. Not all designs are drawable or closed. In addition to this, the arms can be placed on pegs called circle studs, since if both arms are placed there, it makes a circle. There are 6 pictures on a button to cycle through each configuration.

Things which are better about the emulation, is that with the toy, sometimes the crank handle would interfere with the arms. Also, the toy paper would sometimes slip in dry weather or bunch up in humid weather. Also, when the arms jam due to not closed design, the emulation figures out where to restart the design. And some designs may go off the paper, and the emulation has an option to clip or close the design.

Another major thing the emulation adds is called morph, which draws a series of designs with one press of the ADD button. This idea originated with a design sequence in the instructions specified as starting with 10 and stepping up to 13, 16,etc. The emulation uses a base design, a tentative design, and number, and morphs by calculating deltas of each parameter.

There is always a tentative design. *in the future a set of designs which are treated as a group*. In addition to arms and shift lever, there are many more parameters which can be specified. Some can be imagined as physically possible extensions of the toy, and others deal with the idea of placing designs onto a screen, and still others are involved with how the emulation works.

The most basic extension is line width from thin pencil to bold and thick, and color. Magic designer was sold with cheap colored pens.
Once a design is ADD-ed to the suite, the new tentative design is not visible until moved or changed, since it starts with the identical parameters. The emulation will not allow you to add an identical design.

## **How to use the program:**

The program comes with two windows and no conventional menu. The computer screen is divided into two side by side sections. The drawing area on the left and the parameters and functions and modes on the right. You use the mouse in the drawing area depending on the mode. There is move, zoom, bwarp, ewarp, rotate, select, paint, and the temporary modes of slow and bloom.

**Move** tracks the center of the design to any position on the screen. You can even go off the drawing screen by holding down the mouse button and dragging it off.

**Zoom** proportionally expands or shrinks the design. The largest design you can get with the mouse would be to click or drag the mouse as far from center of the design as possible.

**Bwarp and Ewarp** involve changing the location and the size and the vertical and horizontal orientation of the rectangle or square which matches the ellipse or circle which emulates the circular paper of the original toy. By convention, the B point is on the lower left of the screen and the E point is the diagonally opposite from B. The mouse is used to click or drag the corresponding B or E points. It is called warp, since rectangles mean one dimension is compressed.

**B** can have one of 4 orientations which represents the design paper being flipped over either vertically or horizontally. It is like a mirror effect. There is a button which cycles through each corner. The design shape does not change, but the morphing using such designs may change.

The **circle** button makes the radii the same size, removing the warp effect.

**Rotate** allows you to control the orientation of the design frame relative to the screen, emulating the circular paper being tossed onto a table at any angle. You can also drag the mouse in a circular motion and spin the design.

The above modes also use the **U D L R** buttons to fine tune the exact positioning or size.

The coordinates of the center, the size of the radii, and orientation angle in 1/10 of a degree, for the tentative design, are shown.

**Select** mode allows the clicking of the mouse to select the closest design. The selected design is copied to the tentative design by giving it all of its parameters.

Reasons for a selecting a design are:
- as a base for a subsequent morph
- if you like a particular design and do not recall how to specify it
- Turn off the suite switch and use the U or D to see the design suite one at a time.
- used to save a slice of the suite.

**Paint** mode will fill any empty space with the current color, which is the line color of the tentative design. Paint mode only shows the suite and not the tentative design. It colors any space surrounding the mouse click location. It is best to use it in jagged operating mode. Clicking that on or off undoes all of the painting. Load and Delete also undoes painting. Also, paint may not be bounded by the thin line width of 1, and can escape to paint the whole screen. Painting is not saved with either PDF or Save buttons. So to preserve it, do a screenshot. Optionally, you can paint any image derived from the emulation by using a better paint program.

The **Morphing number** is used by ADD to tell it to make a morph design series. It is also used by the U D L R buttons to multiply its effect. It is used by Delete. It is optionally used by Save to save a slice of the suite.

Numeric controls have 4 buttons up or down by 1 and up or down by 50.

**Add** will add one design if there is no designs in the suite or if there is no selected design as a base. Otherwise, it uses the morph number to decide how many designs to add to the suite.

**Delete** uses the morph number to delete that many designs from the suite. If morph is too high, then it deletes the whole suite. Delete is most useful to undo an unintended morph. To do a start over, delete the entire suite.

**It is very important to realize that Add and Delete add to and take from the end of the suite which are the most recently added or loaded designs.**

**Load and Save** adds and saves designs from and to files with the MGS extension. These files have one line for each design. Note that morphed intermediates count as individual designs in the suite. The line format is {n,n, … , n,n} with 26 integers containing parameters for the design. Paint and jagged mode and the size of the drawing screen are not saved.

Save optionally allows one to save a slice from the selected design and morph number is the number to save. If morph is too high, then to the end of the suite.

How to delete the beginning or middle of the suite:
1. Select the first design you wish to save
2. set the morph number large enough to have all of the rest of the designs
3. save a slice of the suite
4. change the morph number to go back to the start of what you want to delete
5. delete
6. load the saved slice

Example: 10 designs, delete 3 thru 6, leaving 1,2,7,8,9,10. Select 7, set morph to 4, save slice 7-10, set morph to 8, delete, load slice.

Editing a design involves setting parameters and positioning the tentative design. And while you can edit any design in the suite, you are only editing a copy.

To edit/replace an actual selected design, do the following steps:
1. if there are any designs past the one you wish to replace - select the next design
2. set the morph number large enough to have all of the rest of the designs
3. save a slice of the suite
4. select the desired design
5. set the morph number to delete that design and the rest of the suite
6. delete
7. edit it
8. add the design
9. load the saved slice

Example, 10 designs and you want to change design 5: select 6, morph 5, Save slice 6-10, Select 5, morph 6, delete, edit it, add, load slice.

**PDF** saves a .pdf file of the screen and its design suite, which can be viewed by any PDF viewer program. Paint and jagged mode is not saved. The size of the drawing screen is saved.

**Bloom** is a movie of the design suite showing it being built.

**Slow** is a movie of a slow motion drawing of the single tentative design. The mouse can be used to pause or kill these movies.

**Arms, shift lever, Line width, and color** are self explanatory if you understand the actual toy.

**T** is the total number of designs. 0 means no designs in the suite.

**S** is the index of the selected design. 0 means no design is selected.

The **picture button** changes the placement of arms according to the design toy. The numerical parameters below that are adjusted accordingly. If you change these parameters, and desire to restore them to normal, cycle through this button. In some placements, the shift lever has no effect on the design. Explaining the details of these parameters, please read the documentation at (http://www.akatz712.com/Physical_parameters.htm)

The concept of **sidedness** of a design is determined by the 3 numbers 6,1,1. You can really have fun with these gear size ratios. I like 13,2,1. The number of rotations needed to do a closed design is calculated based on these numbers up to 99. The physical toy always makes 6 sided designs.

**Pri**me check mark is based on the math of the emulation producing two solutions of how the arms could be positioned. If you placed the arms backwards so that the pen is not on the paper, and were able to draw. It rarely makes anything visually interesting though.

## **Misc options:**

**Clip** is used to emulate whether the circular paper is being emulated. When a design does not fit on the paper, it appears with gaps. The default is no clipping.

**Paper** is used to make a shape. This is used to emulate a colored paper. It will make a square, or rectangle if the Clip is unchecked. Remember to optionally Add a shape before adding the designs placed on it.

**3X plot** is to be used in the rare case when a design is so borderline that it may need more sample points to be done accurately by the emulation. A good way to see if a design needs this is to see if it changes when checking this, or by doing a slow and slow is drawing very fast.

**Jagged** is not a parameter for the design. It tells if the entire suite should be drawn without anti-alias. This is normally used when one plans to paint, since it holds the paint color better.  Checking this removes the paint.

**Suite** is not a parameter. Unchecked if one wants to see the tentative design by itself. This is most useful in using U and D with Select mode. It does not remove the paint.

## Morphing notes:

Differences with parameters based on small numbers or options cannot be used as a morph. Examples are prime, line width, and sidedness.

## Future enhancement:

Sets. The tentative design is now a single design. Find a way to modify a group of designs together.

Other forms of color morphing is an area of possible specification not currently in the program. Right now, Red Green and Blue are primary colors which are morphed individually.

## Other types of curves:

This program can be generalized to emulate other types of curve drawing. For example, something called isotope which was done in Flash language.
Download: isotope3.swf from (www.isotope3.net/isotope3.swf)
Download and extract ```flash_player_sa_linux.x86_64.tar.gz``` from (https://www.adobe.com/support/flashplayer/debug_downloads.html)
Linux : Download the Flash Player Projector (64-bit)

