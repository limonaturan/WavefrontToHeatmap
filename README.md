# WavefrontToHeatmap
Convert a [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file to a heatmap.

The program works best with triangular faces. You can export an obj file using the <i>Triangulate Faces</i> option in [Blender](https://en.wikipedia.org/wiki/Blender_(software)). Make sure that you choose the right coordinate system when exporting to obj. 

## Program help
You can display the help by using the command line argument -h.
```
WavefrontToHeatmap -h
```

## Minimal example
You have to at least specify an input obj file.
```
WavefrontToHeatmap -i Monkey.obj 
```

## Setting the output resoluton
By specifying the command line argument -c you can set the (approximate) pixel count of the output file. The default value is 500000.
```
WavefrontToHeatmap -i Monkey.obj -c 1000
```

![alt text](https://github.com/limonaturan/WavefrontToHeatmap/blob/master/Monkey.png)

