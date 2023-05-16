# AsynchronousGrabQt

# 预先设置
vs打开项目后，需要预先进行以下几项设置才可以正常运行。
1. UI文件夹
ImageLabel.h 和 MainWindow.h 的属性-常规-项类型中，选择Qt meta-Object Complier（moc）
![image](https://github.com/CMakey/AsynchronousGrabQt/assets/39455147/65e7aa72-8f6d-463f-8729-d918ea231dd1)

2. 设置项目属性
项目-属性-VC++目录-包含目录 中，添加程序的本地位置。
![image](https://github.com/CMakey/AsynchronousGrabQt/assets/39455147/588fc439-f00f-4ba0-a795-8e80ee6f5409)

项目-属性-链接器-输入-附加依赖项 中，添加 'VmbImageTransform.lib'
![image](https://github.com/CMakey/AsynchronousGrabQt/assets/39455147/6f9f19b6-a254-4e41-9218-02c7922baeae)


# 从零开始 
如果需要自己构建qt项目，可以先按照本节内容进行设置后，再按照预先设置节的内容进行设置
1. 在vs中新建qt 空项目
- 选择将解决方案与项目放在同一目录中
- qt的设置中，qt model选择Gui，Core，Widgets三个模块，勾选Debug
![image](https://github.com/CMakey/AsynchronousGrabQt/assets/39455147/c42ecc5e-fcca-4e56-a4a6-396c640b37c6)
- 将vimbax的AsynchronousGrabQt文件夹下的内容复制到qt中
- 在文件夹视图下，刷新
2.
