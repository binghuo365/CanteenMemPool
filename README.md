# CanteenMemPool
良好的内存池设计可以显著提高应用程序内存分配效率，一般思路都是一次性向操作系统申请大量内存放入内存池，应用程序需要使用申请内存时，不直接向操作系统要，转而向内存池要，由于内存池由应用程序自己维护，一方面减少与操作系统内核通信代价，另一方面减少内存碎片。
---
大型食堂为提高打饭效率，通常会根据均人次饭量分配很多桌子的饭量，各个桌子以饭碗大小分类，员工吃饭时只需根据自身饭量选择合适饭桌就餐，这样食堂管理员一次准备一桌子饭菜。吃货们只需管饭桌要饭，而饭桌管食堂要饭。
---
CanteenMenPool基于这种思路，内存池管理不同的内存页，应用程序向内存池申请内存时，内存池根据申请内存大小，去合适的内存页分配block即可。
