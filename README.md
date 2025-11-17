<!--
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-08-30 20:51:48
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-09-01 16:14:09
 * @FilePath: \CacheSystem\README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# CacheSystem
使用多个页面替换策略实现了一个线程安全的缓存系统：
+ LRU：最近最久未使用页面置换算法 
+ LFU：最不经常使用页面置换算法
+ ARC：自适应替换缓存算法
