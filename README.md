# Tomasulo 算法模拟器

清华大学计算机系统结构 2020 年春作业。作业要求（Tomasulo.pdf、Example.pdf）、测例（TestCase）、以及个人的实验源码（src）、报告（实验报告.pdf）均已上传。

使用方式参见实验报告。

2020 年的作业要求与 2019 年的有一点不一样。2019 年要求举例：在第 4 周期，第一条 LD 指令执行完后，立刻有一个 Load Unit 变得空闲，第 5 周期应当直接使第三条 LD 进入单元开始执行，而不应当额外再等待一个周期。2020 年将此要求删除，改为需要额外等待一个周期。

最开始直接用 C++ 重写了 https://github.com/Trinkle23897/Undergraduate/tree/master/%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%B3%BB%E7%BB%9F%E7%BB%93%E6%9E%84/tomasulo 的代码（commit id：e23cc6e9），然后进行了一些修改，使得符合今年的要求（latest commit）。
