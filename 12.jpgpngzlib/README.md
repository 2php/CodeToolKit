jpgpngzlib是一个源自QT源码，用于生成jpg、png、zlib的QT工程，通过修改jpgpngzlib.pro中

#uncomment below to make libjpeg
#include(libjpeg.pri)

#uncomment below to make libpng
include(zlib.pri)
include(libpng.pri)


可以编译对应的库文件，非常方便！

by houwenbin1986@gmail.com

