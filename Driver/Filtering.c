#include <Filtering.h>
#include <string.h>

#define N 10 // 滤波窗口大小

int data_array[N]; // 存储窗口内的数据
int sum = 0; // 存储窗口内数据的和
int index = 0; // 当前数据的索引

// 滑动平均滤波器函数
// 参数：new_data - 新的输入数据
// 返回值：滤波后的数据
int Moving_Average_Filter(int new_data) 
{
    sum -= data_array[index]; // 减去窗口中被替换的数据
    data_array[index] = new_data; // 存储新数据
    sum += new_data; // 计算新的和
    index = (index + 1) % N; // 更新索引
    return sum / N; // 返回平均值
}

// 中值滤波器函数
// 参数：new_data - 新的输入数据
// 返回值：滤波后的数据
int Median_Filter(int new_data) 
{
    int sorted_data[N];
    int i, j, temp;

    // 复制数据到临时数组
    for (i = 0; i < N; i++) {
        sorted_data[i] = data_array[i];
    }

    // 冒泡排序
    for (i = 0; i < N - 1; i++) 
		{
      for (j = 0; j < N - i - 1; j++) 
			{
            if (sorted_data[j] > sorted_data[j + 1]) 
						{
                // 交换元素位置
                temp = sorted_data[j];
                sorted_data[j] = sorted_data[j + 1];
                sorted_data[j + 1] = temp;
            }
      }
    }

    data_array[index] = new_data; // 存储新数据
    index = (index + 1) % N; // 更新索引
    return sorted_data[N / 2]; // 返回中值
}
