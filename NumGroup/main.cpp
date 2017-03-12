#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <assert.h>

#include <cstdlib>
#include <cstdio>
#include <vector>

#define NUMBER_COUNT        100
#define GROUP_COUNT         10

#define DISPLAY_COLUMNS     10

typedef signed int   i32;
typedef unsigned int u32;

namespace detail {

int get_random_i32(i32 min_val, i32 max_val)
{
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return (((rand() << 30) & 0x40000000L) | (rand() << 15) | rand())
            % (max_val - min_val + 1) + min_val;
#else
    return ((i32)rand() % (max_val - min_val + 1)) + min_val;
#endif
}

u32 get_random_u32(u32 min_val, u32 max_val)
{
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return ((u32)(((rand() << 30) & 0xC0000000L) | (rand() << 15) | rand()))
            % (max_val - min_val + 1) + min_val;
#else
    return ((u32)rand() % (max_val - min_val + 1)) + min_val;
#endif
}

template <typename T>
void copy_container(T & dest, T const & src) {
    dest.resize(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        dest[i] = src[i];
    }
}

} // namespace detail

thread_local bool RandomSeed_inited = false;

struct RandomSeed {
    RandomSeed() { RandomSeed::init(); }
    ~RandomSeed() { /* Do nothing! */ }

    static void init() {
        if (!RandomSeed_inited) {
            // Generate the randomize seed from time().
            srand((unsigned int)time(NULL));
            RandomSeed_inited = true;
        }
    }
};

template <typename T>
class RandomGenerator {
public:
    typedef T value_type;
    typedef RandomGenerator this_class;

public:
    RandomGenerator() {
        RandomSeed::init();
    }
    ~RandomGenerator() {}

    static value_type next(value_type min_val, value_type max_val) {
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
        return (value_type)(((u32)(((rand() << 30) & 0x40000000L) | (rand() << 15) | rand()))
                % (max_val - min_val + 1) + min_val);
#else
        return (value_type)(((u32)rand() % (max_val - min_val + 1)) + min_val);
#endif
    }

    static value_type next(value_type max_limits) {
        assert(max_limits > 0);
        // Randomize number range is [0, max_limits).
        return this_class::next(0, max_limits - 1);
    }

    static value_type next() {
        // Randomize number range is [0, 2^31).
        return this_class::next(0, 2147483647);
    }

    static i32 next_i32(i32 min_val, i32 max_val) {
        return detail::get_random_i32(min_val, max_val);
    }

    static i32 next_i32(i32 max_limits) {
        assert(max_limits > 0);
        return detail::get_random_i32(0, max_limits - 1);
    }

    static i32 next_i32() {
        // Randomize number range is [0, 2^31).
        return this_class::next_i32(0, 2147483647L);
    }

    static u32 next_u32(u32 min_val, u32 max_val) {
        return detail::get_random_u32(min_val, max_val);
    }

    static u32 next_u32(u32 max_limits) {
        assert(max_limits > 0);
        return detail::get_random_u32(0, max_limits - 1);
    }

    static i32 next_u32() {
        // Randomize number range is [0, 2^32).
        return this_class::next_i32(0, 4294967295UL);
    }
};

struct NumberInfo {
    u32 value;
    u32 index;
};

class Answer {
private:
    struct Item {
        std::vector<NumberInfo> numbers;
        u32 sum, length;
        u32 minimum, maximun;
    };

    u32 length_;

public:
    std::vector<Item> groups;
    std::vector<u32> number_list;

public:
    Answer() : length_(0) {
    }
    Answer(u32 length) : length_(length) {
        init(length, false);
    }
    Answer(u32 length, std::vector<u32> const & numbers) : length_(length) {
        init(length, false);
        set_numbers(numbers);
    }

    ~Answer() {}

    void set_numbers(std::vector<u32> const & numbers) {
        detail::copy_container< std::vector<u32> >(number_list, numbers);
    }

    void resize(u32 length, bool clear = true) {
        init(length, clear);
        length_ = length;
    }

    Item const & get_groups(int index) const {
        assert(index >= 0 && index < (int)groups.size());
        return groups[index];
    }

    void copy_from(Answer const & src) {
        if (this->groups.size() == src.groups.size()) {
            detail::copy_container(this->groups, src.groups);
        }
    }

    void pick_numbers(u32 group_index, i32 index1, i32 index2) {
        assert(group_index >= 0 && group_index < groups.size());
        if (group_index >= 0 && group_index < groups.size()) {
            NumberInfo number_info;
            number_info.value = number_list[index1];
            number_info.index = index1;
            groups[group_index].numbers.push_back(number_info);
            number_info.value = number_list[index2];
            number_info.index = index2;
            groups[group_index].numbers.push_back(number_info);
        }
    }

    void pick_numbers(u32 group_index, i32 index) {
        assert(group_index >= 0 && group_index < groups.size());
        if (group_index >= 0 && group_index < groups.size()) {
            NumberInfo number_info;
            number_info.value = number_list[index];
            number_info.index = index;
            groups[group_index].numbers.push_back(number_info);
        }
    }

    void sort_numbers(bool is_asc = true) {
        for (size_t g = 0; g < groups.size(); ++g) {
            std::vector<NumberInfo> & numbers = groups[g].numbers;
            if (is_asc) {
                for (size_t i = 0; i < numbers.size() - 1; ++i) {
                    for (size_t j = i + 1; j < numbers.size(); ++j) {
                        if (numbers[i].value > numbers[j].value) {
                            std::swap(numbers[i], numbers[j]);
                        }
                    }
                }
            }
            else {
                for (size_t i = 0; i < numbers.size() - 1; ++i) {
                    for (size_t j = i + 1; j < numbers.size(); ++j) {
                        if (numbers[i].value < numbers[j].value) {
                            std::swap(numbers[i], numbers[j]);
                        }
                    }
                }
            }
        }
    }

private:
    void init(u32 length, bool clear) {
        if (clear) {
            groups.clear();
        }
        groups.resize(length);

        u32 capacity = length * 3;
        for (size_t i = 0; i < length; ++i) {
            if (clear) {
                groups[i].numbers.clear();
            }
            groups[i].numbers.reserve(capacity);
            groups[i].sum = 0;
            groups[i].length = 0;
            groups[i].minimum = 2147483647;
            groups[i].maximun = 0;
        }
    }    
};

namespace detail {

void display_numbers(std::vector<u32> const & numbers, i32 columns)
{
    u32 length = (u32)numbers.size();
    printf("Numbers[%u] = {\n", length);
    for (u32 i = 0; i < length; ++i) {
        if (((i32)i % columns) == 0)
            printf("    ");
        if (i < length - 1)
            printf("%3d, ", numbers[i]);
        else
            printf("%3d", numbers[i]);
        if (((i32)i % columns) == (columns - 1))
            printf("\n");
    }
    printf("}\n\n");
}

void display_answer_detail(Answer const & answer, u32 sum, u32 length, i32 groups)
{
    double average = (double)sum / groups;
    double std_diff = 0.0;
    i32 n_diff = 0, n_average = (i32)sum / groups;

    printf("Answer[%u] = {\n", length);
    for (i32 g = 0; g < groups; ++g) {
        printf("    group[%2d]: { ", g + 1);
        u32 sum = 0;
        size_t group_length = answer.groups[g].numbers.size();
        if (group_length != 0) {
            for (size_t i = 0; i < group_length; ++i) {
                u32 value = answer.groups[g].numbers[i].value;
                if (i < group_length - 1)
                    printf("%3d, ", value);
                else
                    printf("%3d", value);
                sum += value;
            }
        }
        else {
            printf(" <Empty> ");
        }
        double diff = (double)sum - average;
        std_diff += diff * diff;
        n_diff += sum - n_average;
        printf(" }, sum = %u.\n", sum);
    }
    printf("}\n\n");

    printf("group average: %0.3f, standard diff: %0.3f, average diff: %0.3f .\n\n",
           average, std_diff / groups, (double)n_diff / groups);
}

} // namespace detail

class NumberGroup {
private:
    typedef RandomGenerator<u32> RandomGen;

    u32 length_, groups_;
    u32 min_num_, max_num_;
    u32 sum_, average_, remain_;
    std::vector<u32> numbers_;
    std::vector<u32> sorted_list_;
    Answer answer_;

public:
    NumberGroup(u32 length, u32 groups, u32 min_num, u32 max_num)
        : length_(length), groups_(groups), min_num_(min_num), max_num_(max_num),
          sum_(0), average_(0), remain_(0) {
        // Generate randomize numbers.
        generate_numbers(length, min_num, max_num);
    };
    ~NumberGroup() {}

    u32 length() const { return length_; }
    u32 groups() const { return groups_; }
    std::vector<u32> const & numbers() const { return numbers_; }

    void display_numbers() {
        detail::display_numbers(numbers_, DISPLAY_COLUMNS);
    }

    void display_sorted_numbers() {
        detail::display_numbers(sorted_list_, DISPLAY_COLUMNS);
    }

    void display_answers_detail() {
        detail::display_answer_detail(answer_, sum_, length_, groups_);
    }

    void display_answers() {
        printf("The answers is:\n\n");
        display_answers_detail();
    }

    void display_no_answers() {
        printf("No answers!\n\n");
    }

    void get_basic_info() {
        u32 sum = 0;
        for (size_t i = 0; i < numbers_.size(); ++i) {
            sum += numbers_[i];
        }
        sum_ = sum;
        average_ = sum / groups_;
        remain_ = sum % groups_;

        double group_average = (double)sum / groups_;
        double num_average = (double)sum / length_;
        printf("sum = %u, remain = %u, group_average = %0.3f, num_average = %0.3f .\n\n",
               sum_, remain_, group_average, num_average);
    }

    void sort_numbers() {
        detail::copy_container< std::vector<u32> >(sorted_list_, numbers_);

        size_t i, j;
        size_t length = numbers_.size();
        for (i = 0; i < length - 1; ++i) {
            for (j = i + 1; j < length; ++j) {
                if (sorted_list_[i] < sorted_list_[j]) {
                    std::swap(sorted_list_[i], sorted_list_[j]);
                }
            }
        }
    }

    int balance_pick_numbers(Answer & result) {
        u32 step = length_ / 2;
        u32 group_index = 0;

        i32 front, back;
        front = length_ / 2 - 1;
        if (front < 0)
            front = 0;
        back = front + 1;
        for (u32 i = 0; i < step; ++i) {
            group_index = RandomGen::next(groups_);
            result.pick_numbers(group_index, front, back);
            group_index++;
            group_index %= groups_;
            front--;
            back++;
            if (front < 0 || back >= (i32)length_)
                break;
        }

        if (front < 0 && back < (i32)length_) {
            result.pick_numbers(group_index, back);
        }
        else if (front >= 0 && back >= (i32)length_) {
            result.pick_numbers(group_index, front);
        }
        
        return 1;
    }

    int random_pick_numbers(Answer & result) {
        i32 remain_length = (i32)length_;
        while (remain_length >= 0) {
            u32 group_index = RandomGen::next(groups_);
            i32 rand_index = RandomGen::next_i32(remain_length);
            result.pick_numbers(group_index, rand_index);
        }
        return 0;
    }

    int slove() {
        get_basic_info();

        sort_numbers();
        display_sorted_numbers();

        answer_.resize(groups_);
        answer_.set_numbers(sorted_list_);
        
        Answer answer(groups_, sorted_list_);
        int results = random_pick_numbers(answer);
        if (results > 0) {
            answer_.copy_from(answer);
        }
        answer_.sort_numbers(false);
        return 1;
    }

private:
    void generate_numbers(u32 length, u32 min_num, u32 max_num) {
        numbers_.resize(length);

        if (min_num > max_num) {
            std::swap(min_num, max_num);
        }
        for (u32 i = 0; i < length; ++i) {
            numbers_[i] = RandomGen::next_u32(min_num, max_num);
        }
    }
};

int main(int argc, char * argv[])
{
    RandomSeed::init();

    NumberGroup slover(NUMBER_COUNT, GROUP_COUNT, 5, 100);

    slover.display_numbers();

    int answers = slover.slove();
    if (answers > 0) {
        slover.display_answers();
    }
    else {
        slover.display_no_answers();
    }

    ::system("pause");
    return 0;
}
