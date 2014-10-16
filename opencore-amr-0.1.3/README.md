解压 opencore-amr-0.1.3.tar.gz 到当前目录： opencore-amr-0.1.3

先进入源码目录（下面configure配置按照自己的路径来设置）：


x86编译：

make clean -w

./configure --prefix='/home/lijie/txaa/ubiq_proj/amr/opencore-amr-0.1.3/x86'

make

make install 


arm编译：

make clean -w

./configure --host=arm-hisiv100nptl-linux --prefix='/home/lijie/txaa/ubiq_proj/amr/opencore-amr-0.1.3/hisi'

make

make install 
