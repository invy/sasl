Let's Look at texture component closely. It consists of single function call:

```
drawTexture(get(image), 0, 0, 100, 100)
```

drawTexture is function which works exactly as name states.  First argument is texture to draw.  Next two number is X and Y coordinates of desired texture location.  In this case texture drawn starting from component corner (0, 0).  Next two numbers are width and height of desired image.  You can set any number and texture will be scaled automatically.  In this example entire of component size filled by texture (100, 100).

When drawTexture renders texture it uses mixing with some background color.  If background color is not specified (like in example above) drawTexture automatically gets background color from simulator.  It can be just white (texture will be drawn as is) or something other to simulate night vision.

Let's examine textureLit component.  It almost the same but background color defined:

```
drawTexture(get(image), 0, 0, 100, 100, 1, 1, 1)
```

drawTexture accepts 4 numbers: red, green, blue and alpha components of color.  (1, 1, 1, 1) states for white non-transparent color.

In most situations it will be enough to use texture component.  You texture will be automatically faded in night and lighted by flood light in cockpit.  But if you'd like to add something like warning lamp you have to use textureLit component.  It will ignore default background color and will be always drawn 'as is'.

If you need more complex lighting logic feel free to make your own texture components using textureLit as reference.