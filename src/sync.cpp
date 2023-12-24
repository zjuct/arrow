#include <sync.hpp>



// template <typename T, typename... args>
// FuncSyncPackage::FuncSyncPackage(FuncType funcType, T *param, args... arg)
// {
//     type = funcType;
//     timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//     size = 0;
//     getSize(param, arg...);
//     data = new char[size];
//     init(0, param, arg...);
// }

// template <typename T, typename... args>
// void FuncSyncPackage::init(int offset, T *param, args... arg)
// {
//     memcpy(data + offset, param, sizeof(T));
//     init(offset + sizeof(T), arg...);
// }

// template <typename T, typename... args>
// void FuncSyncPackage::getSize(T *param, args... arg)
// {
//     size += sizeof(T);
//     getSize(arg...);
// }
// template <typename T, typename... args>
// void FuncSyncPackage::get(T* param, args... arg)
// {
//     getfrom(0, param, arg...);
// }

// template <typename T, typename... args>
// void FuncSyncPackage::getfrom(int offset, T *param, args... arg)
// {
//     memcpy(param, data + offset, sizeof(T));
//     getfrom(offset + sizeof(T), arg...);
// }