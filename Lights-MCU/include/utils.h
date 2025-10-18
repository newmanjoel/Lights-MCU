#ifndef utils
#define utils

template <typename T, size_t N>
inline void clear_volatile_array(volatile T (&arr)[N], T value = 0) {
    for (size_t i = 0; i < N; ++i)
        arr[i] = value;
}


#endif // utils