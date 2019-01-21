# WavefrontToHeatmap
Convert a [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file to a heatmap.

The program works best with triangular faces. You can export an obj file using the <i>Triangulate Faces</i> option in Blender. Make sure that you choose the right coordinate system when exporting to obj.

## Program help
You can display the help by using the command line argument -h.
```
WavefrontToHeatmap -h
```

## Example
```
WavefrontToHeatmap -i file.obj 
```

![alt text](https://github.com/limonaturan/WavefrontToHeatmap/blob/master/Monkey.png)

