1、upnp、dlna的基础知识文档：

   http://wenku.baidu.com/view/235374d380eb6294dd886c44.html
   http://wenku.baidu.com/view/857f2638580216fc710afd04.html
   http://wenku.baidu.com/view/87795b134431b90d6c85c763.html
   http://wenku.baidu.com/view/04616addce2f0066f533226d.html
   http://www.cnblogs.com/swordzj/archive/2012/03/18/2404840.html

2、vs2012工程说明：

   mydlnaconsoletest能在win8下启动我们的cp\dms\dmr, 启动代码为mydlna.cpp，启动后在intel的工具里能侦测到他们，我们的cp也能侦测到新的PlatinumKit写的dmr等；
   其依赖的Neptune.lib可以由下面的说明里生成出来。

   Hilo为win8 metro风格的dlna界面， 已经结合dlna库代码到工程里。

3、其他：

   IntelToolsForUPnPTechnology_v2、IntelDigitalHomeDeviceCodeWizard为intel的upnp及dlna工具包， 里边的devicespy、av media control等工具比较有用。

   PlatinumKit.rar包里的Platinum\Build\Targets\x86-microsoft-win32-vs2008或2010目录下等工程能直接编译过, 其   MicroMediaController、MediaRendererTest、
   FileMediaServerTest即是cp\dmr\dms的实例， 先启动MicroMediaController(cp)，  再用MediaRendererTest -f mydmr、   FileMediaServerTest -f mydms c:\\temp
   (自设目录， 里边有图片等资源文件等供调用)等命令启动dms\dmr，在MicroMediaController输入help看命令帮助信息， 输入setms等可以控制dms等， 在intel的工具里
    也能看到启动的dms等，SimpleTest为简单upnp设备测试。

   IPadAndIPhone里的mymediadlna包为IPadAndIPhone的dlna代码包， 里边有一个Neptune目录，有针对我们的dlna包依赖Neptune的一些修改， 标准的Neptune包能编译过后
   直接覆盖里边的文件过去可以生成我们的特殊Neptune.lib。
