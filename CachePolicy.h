/*
 * @Author: hayden 2867571834@qq.com
 * @Date: 2025-09-09 21:39:36
 * @LastEditors: hayden 2867571834@qq.com
 * @LastEditTime: 2025-10-23 15:29:10
 * @FilePath: \CacheSystem\CachePolicy.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

namespace CacheSystem
{
template <typename Key, typename Value>
class CachePolicy
{
public:
    virtual ~CachePolicy() {};

    // 添加缓存接口
    virtual void put(Key key, Value value) = 0;

    // key是传入参数  访问到的值以传出参数的形式返回 | 访问成功返回true
    virtual bool get(Key key, Value& value) = 0;
    // 如果缓存中能找到key， 则直接返回value
    virtual Value get(Key key) = 0;
};

}