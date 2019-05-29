# convert - a file format conversion tool

This tool is a project on behalf of a C++ programming lecture at the Humboldt University Berlin.

The task was to implement a file format converting tool between the following formats:

* PPM image format
* RLE image format
* Huffman encoded raw file
* RLE+Huffman encoded image file
* raw file (no format)

from any format of the above into any format of the above.

Clear is, that PPM image format and RLE as image format don't play nice together when used as target
with a source file that is not an image. But task is task. Points are points.

## Usage

```
convert - command line tool for converting some file formats.
Copyright (c) 2019 by Christian Parpart and Kei Thoma.

Options:
  -I, --input-format=FORMAT
                                Specifies which format the input stream has. [raw]
  -i, --input-file=PATH
                                Specifies the path to the input file to read from.
  -O, --output-format=FORMAT
                                Specifies which format the output stream will be. [raw]
  -o, --output-file=PATH
                                Specifies the path to the output file to write to.
  -h, --help
                                Shows this help.
```
