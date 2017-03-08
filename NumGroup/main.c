
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>

#define NUMBER_COUNT   100
#define GROUP_COUNT    10

typedef struct _number_info {
    unsigned int value;
    unsigned int index; 
    int          group;
} number_info;

typedef struct _group_item {
    struct group_item *  next;
    struct number_info * info;
} group_item;

typedef struct _group_info {
    unsigned int capacity;
    unsigned int length;
    unsigned int sum;
    struct number_info * numbers[1];
} group_info;

void swap_i32(int * a, int * b)
{
    int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void swap_u32(unsigned int * a, unsigned int * b)
{
    unsigned int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void swap_number_info(number_info * info1, number_info * info2)
{
    swap_u32(&info1->value, &info2->value);
    swap_u32(&info1->index, &info2->index);
    swap_i32(&info1->group, &info2->group);
}

int get_random_i32(int min_num, int max_num)
{
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return (((rand() << 30) & 0x40000000L) | (rand() << 15) | rand())
            % (max_num - min_num + 1) + min_num;
#else
    return (rand() % (max_num - min_num + 1)) + min_num;
#endif
}

unsigned int get_random_u32(unsigned int min_num, unsigned int max_num)
{
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return ((unsigned int)(((rand() << 30) & 0xC0000000L) | (rand() << 15) | rand()))
            % (max_num - min_num + 1) + min_num;
#else
    return ((unsigned int)rand() % (max_num - min_num + 1)) + min_num;
#endif
}

number_info * generate_numbers(unsigned int length,
                               unsigned int min_num,
                               unsigned int max_num)
{
    number_info * numbers = (number_info *)malloc(sizeof(number_info) * length);
    if (numbers == NULL)
        return NULL;

    if (min_num > max_num) {
        swap_u32(&min_num, &max_num);
    }
    for (unsigned int i = 0; i < length; ++i) {
        numbers[i].value = get_random_u32(min_num, max_num);
        numbers[i].index = i;
        numbers[i].group = -1;
    }

    return numbers;
}

void display_numbers(number_info * numbers, unsigned int length, int columns)
{
    printf("Numbers[%u] = {\n", length);
    for (unsigned int i = 0; i < length; ++i) {
        if (((int)i % columns) == 0)
            printf("    ");
        if (i < length - 1)
            printf("%3d, ", numbers[i].value);
        else
            printf("%3d", numbers[i].value);
        if (((int)i % columns) == (columns - 1))
            printf("\n");
    }
    printf("}\n\n");
}

void display_result(number_info * numbers, unsigned int length, int groups)
{
    printf("Result[%u] = {\n", length);
    unsigned int sum;
    for (int g = 0; g < groups; ++g) {
        printf("    group[%2d]: { ", g + 1);
        sum = 0;
        for (unsigned int i = 0; i < length; ++i) {
            if (numbers[i].group == g) {
                if (i < length - 1)
                    printf("%3d, ", numbers[i].value);
                else
                    printf("%3d", numbers[i].value);
                sum += numbers[i].value;
            }
        }
        printf(" }, sum = %u.\n", sum);
    }
    printf("}\n\n");
}

void sort_numbers_desc(number_info * numbers, unsigned int length)
{
    unsigned int i, j;
    for (i = 0; i < length - 1; ++i) {
        for (j = i + 1; j < length; ++j) {
            if (numbers[i].value < numbers[j].value) {
                swap_number_info(&numbers[i], &numbers[j]);
            }
        }
    }
}

int get_over_averages(number_info * numbers, unsigned int length, unsigned int groups, unsigned int average)
{
    int group_index = 0;
    for (unsigned int i = 0; i < length; ++i) {
        if (numbers[i].value >= average) {
            numbers[i].group = group_index % groups;
            group_index++;
        }
        else break;
    }
    return group_index;
}

int is_final_result(group_info * group_list, unsigned int length, unsigned int groups,
                    unsigned int averages, unsigned int remain)
{
    unsigned int i, total;

    total = 0;
    for (i = 0; i < length; ++i) {
        if (group_list[i].length == 0)
            return 0;
        total += group_list[i].length;
    }
    if (total != length)
        return 0;

    int diff0 = 0, diff1 = 0;
    for (i = 0; i < length; ++i) {
        if (group_list->sum == averages)
            diff0++;
        if (group_list->sum == averages + 1)
            diff1++;
    }

    // If all of diff0 = 0, or diff1 = remain, it's the final answer.
    if ((remain == 0 && diff0 == groups) ||
        (remain != 0 && diff1 == remain && diff0 == (groups - remain))) {
        return 1;
    }
    return 0;
}

int random_pick_numbers()
{
    //
    return 0;
}

int slove_number_group_impl(number_info * numbers, unsigned int length, unsigned int groups,
                            unsigned int averages, unsigned int remain, int over_averages)
{
    unsigned int capacity = groups * 3;
    group_info * group_list = (group_info *)malloc(sizeof(group_info) + sizeof(number_info) * (capacity - 1));
    if (group_list == NULL)
        return -1;

    unsigned int i, j;
    int pick_nums;
    int is_final;
    double best_diff = 0.0;

    do {
        for (i = 0; i < length; ++i) {
            group_list[i].capacity = capacity;
            group_list[i].length = 0;
            group_list[i].sum = 0;
            for (j = 0; j < length; ++j) {
                group_list[i].numbers[j] = NULL;
            }
        }
    
        pick_nums = 0;
        while (pick_nums != length || !is_final_result(group_list, length, groups, averages, remain)) {
            // Randomize pick 1/3 to 1/2 of total numbers randomize throw to each groups.
            random_pick_numbers(group_list, length);
        }

        is_final = is_final_result(group_list, length, groups, averages, remain);
    } while (!is_final);
    return 0;
}

number_info * slove_number_group(number_info * numbers, unsigned int length, unsigned int groups)
{
    unsigned int sum = 0, remain;
    double average = 0.0;
    number_info * result = NULL;

    // Calculate the sum and average.
    for (unsigned int i = 0; i < length; ++i) {
        sum += numbers[i].value;
    }
    average = (double)sum / groups;
    remain = sum % groups;
    printf("sum = %u, average = %0.3f, remain = %u.\n\n", sum, average, remain);

    sort_numbers_desc(numbers, length);

    // Display sorted (desc) numbers to screen.
    display_numbers(numbers, NUMBER_COUNT, 10);

    int over_averages = get_over_averages(numbers, length, groups, (unsigned int)average);
    printf("over_averages = %d\n\n", over_averages);

    result = (number_info *)malloc(sizeof(number_info) * length);
    for (unsigned int i = 0; i < length; ++i) {
        result[i] = numbers[i];
    }

    int success = slove_number_group_impl(result, length, groups,
                                          (unsigned int)average,
                                          remain, over_averages);
    if (success <= 0) {
        if (result) {
            free(result);
            result = NULL;
        }
    }

    printf("\n");
    return result;
}

int main(int argc, char * argv[])
{
    // Generate the randomize seed from time().
    srand((unsigned int)time(NULL));

    number_info * numbers = generate_numbers(NUMBER_COUNT, 5, 100);
    if (numbers != NULL) {
        // Display numbers to screen.
        display_numbers(numbers, NUMBER_COUNT, 10);

        number_info * result = slove_number_group(numbers, NUMBER_COUNT, GROUP_COUNT);
        if (result != NULL) {
            //
            printf("The answer is: \n\n");

            // Display result numbers to screen.
            display_result(result, NUMBER_COUNT, GROUP_COUNT);
            printf("\n");
        }
        else {
            printf("Have no answer!\n\n");
        }
        if (result)
            free(result);
    }

    if (numbers)
        free(numbers);
   
    return 0;
}
