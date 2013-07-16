WTV - DDS file viewer

Overview
--------
WTV was developed for simple and fast DDS viewing, escpecially for those DDS files with alpha channel,
because there where no free or even commercial viewers/browsers which could show an image modulated by it's alpha channel.

Supported DDS pixel formats
---------------------------
* 8888
* 888
* 565
* 555
* 1555
* 4444
* DXT1/A
* DXT3
* DXT5
* DXT5_NM
* V8U8
* x8 ( G8, B8 not tested yet )
* R16F
* G16R16F
* A16B16G16R16F
* R32F
* G32R32F
* A32B32G32R32F
* ATI1N, ATI2N
* G16R16

Supported map types
-------------------
* 2D
* Cube
* Volume

Installation
-----------
Copy WTV.EXE to a permanent destination (e.g. C:\Utils\Wtv\ in my case)
From version 0.58b, you can associate WTV with DDS files through the menu option "Help->Associate DDS file with WTV"
If it doesn't work, try manual association. This way has been tested with WinXP:
- right click on any DDS file
- select "Open With"
- select "Choose program..."
- click on "Browse" button and find the WTV app.
- click on "Open" button
- check "Always use the selected program to open this kind of file"
- click on "OK" button

How to use WTV
--------------
The fastest way of using WTV is to associate it with DDS files.
Then, simply double-click or press enter on any DDS file to view it.
Another way is to use Drag&Drop or the good old "File->Open..."
By default, the image will be modulated by it's alpha channel ( if there is one ).
If you want to disable that function, press "U" or click on menu->options->Use alpha channel.
Background color is pink by default ( can be changed ), so transparent pixels will be pink.
To view alpha channel only press "A" or click on menu->options->Show alpha channel.
You can browse through all DDS files in the same folder by the mouse wheel rotation, up/down or pgup/pgdn keys.
+/- keys or mouse wheel zooms the image. When the mouse cursor is changed to drag, the image is bigger than window and can be moved
by right-mouse button + mouse move.
Left/right keys change current mipmap and Z/X keys change current side/layer for cube/volume maps.

Control keys overview
---------------------
Up/Down, PgUp/PgDn -> previous/next picture
Home -> first picture in the folder
End -> last picture in the folder
Delete -> deletes current file and move to the next one
Mouse wheel -> previous/next picture or zoom. Depends on settings
Left/Right -> change mipmap
+/- -> zoom in / zoom out
* -> reset zoom to 100%
A -> show/hide alpha channel. Use alpha to blend if any other channel is shown.
R -> show/hide red channel
G -> show/hide green channel
B -> show/hide blue channel
U -> show just alpha or RGB
B -> specify background color
C -> ON/OFF window clipping to nearest monitor work area
Z/X -> change side (cube maps) or change layer (volume maps)
Right mouse button press + mouse move -> move image which is bigger than the window

Status bar information
----------------------
Type - is fixed to DDS in current version. I plan at least TGA support.
Format - pixel format of the current DDS file
Size - size of the current file mipmap
Layer/Side - LAYER = current volume layer / layer count, SIDE = current cube map side / sides used
MipMap - current mipmap / mipmap count
Mem - memory taken by current mipmap / memory taken by all mipmaps together

Contact
-------
Tomas (Woody) Blaho
Illusion Softworks
tomas.blaho@illusionsoftworks.com
2KCzech
tomas.blaho@2kczech.com

!! Any suggestions will be appreciated. !!

Version history
---------------
0.89b - Fixed ARGB info for selected pixel.
      - Luminance format names fixed.
0.88b - Added drag source part for "Drag & Drop". It's now possible to drag&drop browsed file from WTV to Photoshop with a DDS plugin installed to open the DDS file there.
0.87b - Added ARGB info for a selected pixel
0.86b - Fixed hang in overlapped file handling on Windows XP 64bit
      - Fixed damaged DXT5 image after ATI2N image was viewed
0.85b - Changed format name generation to support more than 8bits per channel.
      - Added support for G16R16 format.
      - Fixed blending of A8L8 format.
      - Fixed menu channel selection.
0.84b - Fixed ATI2N format decompression.
      - Added support for ATI1N format
0.83b - Any single channel is shown as gray scale
      - shortcuts added for forced single channel view. It is shift or ctrl + R or G or B or A.
      - all channels are shown after shift or ctrl + U. The U alone switches between A and RGB channels.
0.81b - More normal map formats shown through the new PostProcess option.
      - fixed name and presentation of A8L8 format.
0.80b - Fixed path to the current, when file name used as a parametr desn't have path included.
      - Added support for ATI2N format
      - Added support for a separate channel view. Shortcuts slightly redesigned.
      - Delete key deletes current file and move to next one.
0.70b - Fixed image movement outside the main monitor
0.69b - Added chance to remove DDS file association (menu Help->Remove association of DDS files with WTV)
0.68b - Fixed float CUBE/VOLUME maps.
      - float format pixels drawing bound to (0.0f .. 1.0f) range.
0.67b - Added conversion to long file name in case the system calls WTV with the short filename as parameter.
0.66b - Added "Treat DXT5 as DXT5_NM" possibility (menu item or 'n' key)
      - Fixed alpha usage status after WTV starts
0.65b - HOME key leads to the first picture in the current folder, END key leads to last picture in the current folder
      - zoom by keys '+' and '-' is now in predefined way: 1%, 5%, 10%, 15%, 20%, 30%, 50%, 70%, 100%, 150%, 200%, 300%, 
        500%, 700%, 1000%, 2000%, 3000%, 5000% and 10000%. So when the zoom is 110% and the '+' is pressed, it jumps 
        to 200% and not to (2 * currentZoom) = 220%. When the '-' is pressed, it jumps to 100% and not to (0.5 * currentZoom) = 60%
      - Fixed the rounding bug in the current percentual zoom level information in the caption bar.
0.64b - A8 (or others "just 8bit per pixel" formats) now works well also in "A" and "ARGB" modes. Not only in "RGB" mode.
      - Fixed pixel format description for formats with an alpha channel ("32bit - X8R8G8B8" instead of "32bit - R8G8B8" etc.)
0.63b - Fixed background cleaning on some situations.
      - Added "Always on top" possibility
      - Added "Filter image" settings for shrinking and enlarging
      - Fixed "Clip to work area" for multi-monitor systems. It's now renamed to "Clip to nearest monitor"
      - Added "Always in center of nearest monitor" possibility
      - Added "Auto zoom" possibility, which ensure that the full image is always displayed. Not just it's part.
      - Added "Wrap arround while changing files" possibility
      - Fixed progress bar visibility when just alpha channel has been displayed.
0.62b - ESC evokes exit on press, not on release.
      - Fixed ESC while open or color dialog is visible. It also closed whole WTV.
      - Zoom possibility for mouse wheel added in menu "Options" -> "Mouse wheel behaviour" (thx to Doane Rhoag)
      - Progress bar now appears after 250ms instead of 500ms
      - Progress bar now shows not only loading progress.
        It is still only one per image, but it has three parts. 
        0 -> 33% file loading, 
        33% -> 66% decompression (skipped for uncompressed images)
        66% -> 100% blending alpha with color + swapping lines
      - small registry saving fixies
0.61b - Fixed program hang after the file of given file name was not found.
      - Fixed rare problem in listing files.
      - Fixed loading of non-standard mimmaps in old DDS files
      - DDS files with unsupported pixel format do not create message box. There is just cleared window and "Format Unknown" text in status bar.
0.60b - asynchronous loading implemented, so very large textures can be skipped during loading. 
        Also mipmap/vol.layer/cube-side can be changed, instead of waiting until the end of current loading.
      - loading progress bar added. It appears automatically after 500ms of loading, so it doesn't disturb for short loadings.
0.59b - fixed cube maps without mipmaps
      - minor fix in window size computation
0.58b - automatic DDS file association added (menu Help->Associate DDS files with WTV)
0.57b - volume/cube maps support ('z'/'x' key changes "depth layer"/"cube side")
      - fixed crash for folders with thousands of DDS files (thx to Doane Rhoag)
      - fixed memory size indicator for current mipmap
0.56b - DDS decoder improved (very small mipmaps, different DDS headers)
0.55b - 16bit and 32bit floating point DDS pixel format support added.
      - backgroud color can be chosen through the dialog
      - channel usage/visibility information added to menu
      - last window pos, background color, use/show alpha, window clipping and used path are saved to the registry
      - key '*' resets zoom to 100%
0.52b - 8bit DDS pixel format support added. Only A8 and R8 were tested.
0.5b  - "File->Open..." and Drag & Drop method implemented (thx to Doug Rogers)
      - minor bug fixes
0.03b - First usable version.