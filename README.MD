### 关于源程序

本人仅在 flappy bird c++ 版的原作上加入一个pipe用于训练数据的传输，关于 flappy bird c++ (VS2012)，由于我找不到原作品的相关信息和链接，在此表示非常抱歉，可阅读原作者相关的 README。

### 关于AI 

#### 文件结构

AI 程序为单个 CPP 程序，相关**宏定义**保持与原作品中相同。为了节约Github的空间，已将`fancy2D`压缩，`clone` 后请解压，并保持一下文件结构，以便正确编译。

**File Structure：**

	`
	|-- FlappyBird  
	|	|-- fancy2D              // 游戏引擎库
	|	|	|-- fancy2D
	|	|	|-- fancylib
	|	|	|-- fancyUI
	|	|-- FlappyBird           // 源代码，加入pipe传状态数据
	|		|-- FlappyBird/Data  // 游戏素材
	|		|-- Readme.txt       // 原作者的相关说明
	|
	|-- FlappyBird_Qlearning
	|	|-- qlearning.cpp        // AI using Q-Learning
    | 	|-- Q4218222.txt         // 一个已训练好的 action-value function
	|
	|-- README.MD
	

#### 运行时库

独立运行时，请将游戏引擎相关 `fancy2D.dll` 以及系统相关 `D3DX9_43.dll`， `msvcp11.dll`， `msvcr110.dll`，以及游戏素材 `Data` 拷贝至同一文件夹。


#### 启动顺序

由于管道创建原因，训练时请先启动游戏，后启动AI。

#### 说明文档

原理和相关细节详见[此处](http://jiehust.github.io/machine%20learning/2015/01/26/Playing-Flappy-Bird-using-Q--learning/)