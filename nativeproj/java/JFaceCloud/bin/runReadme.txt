run.sh 用xinit 调用runx.sh
runx.sh里调起java 

run.sh runx.sh 会有权限问题 执行前要先chmod +x *改变一下权限



阿里云开机运行TOMCAT WITH X SERVER

设置/etc/rc.local

startx
exit 0


设置/etc/profile
mkdir /tmp/yuji
export DISPLAY=:0
./face/apache-tomcat-8.0.36/bin/startup.sh

