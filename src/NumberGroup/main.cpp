
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <assert.h>

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "get_char.h"

#define NUMBER_COUNT        100
#define GROUP_COUNT         10

#define DISPLAY_COLUMNS     10

#define PRE_PICKUP_COFFE        (0.6f)
#define GROUP_SUM_LIMIT_COFFE   (0.6f)

#define noif(statement) \
    do { \
        statement; \
    } while (0);

typedef signed int   i32;
typedef unsigned int u32;

static int g_result_cnt = 0;

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

static thread_local bool RandomSeed_inited = false;

struct RandomSeed {
    RandomSeed() { RandomSeed::init(); }
    ~RandomSeed() { /* Do nothing! */ }

    static void init(int seed = -1) {
        if (!RandomSeed_inited) {
            if (seed == -1) {
                // Generate the randomize seed from time().
                srand((unsigned int)time(NULL));
            }
            else {
                srand((unsigned int)seed);
            }
            RandomSeed_inited = true;
        }
    }

    static void force_reinit(int seed = -1) {
        if (seed == -1) {
            // Generate the randomize seed from time().
            srand((unsigned int)time(NULL));
        }
        else {
            srand((unsigned int)seed);
        }
        RandomSeed_inited = true;
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
        return this_class::next(0, 2147483647L);
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
//  u32 index;
};

class Answer {
private:
    struct Group {
        std::vector<NumberInfo> numbers;
        u32 sum;
        i32 length;
        u32 minimum, maximun;
    };

    i32 group_max_;
    i32 num_length_;
    i32 num_used_;
    u32 sum_;
    u32 average_;
    u32 remain_;
    u32 sum_limit_;

public:
    std::vector<Group> groups;
    std::vector<u32> number_list;

    static const double kSumLimitCoffe;

public:
    Answer() : group_max_(0), num_length_(0), num_used_(0),
               sum_(0), average_(0), remain_(0), sum_limit_(0) {
    }

    Answer(i32 n_groups, i32 num_length)
        : group_max_(n_groups), num_length_(num_length), num_used_(0),
          sum_(0), average_(0), remain_(0), sum_limit_(0) {
        init(n_groups, false);
        get_basic_info();
    }

    Answer(i32 n_groups, i32 num_length, std::vector<u32> const & numbers)
        : group_max_(n_groups), num_length_(num_length), num_used_(0),
          sum_(0), average_(0), remain_(0), sum_limit_(0) {
        init(n_groups, false);
        copy_numbers(numbers);
    }

    Answer(const Answer & src) : Answer() {
        copy_from(src);
    }

    ~Answer() {}

    i32 group_max() const { return group_max_; }
    i32 num_length() const { return num_length_; }
    std::vector<u32> const & numbers() const { return number_list; }

    u32 sum() const { return sum_; }
    u32 average() const { return average_; }
    u32 remain() const { return remain_; }
    u32 sum_limit() const { return sum_limit_; }

    i32 used_numbers() const { return num_used_; }
    i32 remain_numbers() const { return (num_length_ >= num_used_) ? (num_length_ - num_used_) : (num_used_ - num_length_); }

    void copy_numbers(std::vector<u32> const & numbers, i32 num_length = -1) {
        detail::copy_container< std::vector<u32> >(number_list, numbers);
        if (num_length < 0)
            num_length_ = (u32)number_list.size();
        else
            num_length_ = (u32)num_length;
        get_basic_info();
    }

    void set_basic_info(u32 sum, u32 average, u32 remain) {
        sum_ = sum;
        average_ = average;
        remain_ = remain;
        sum_limit_ = get_sum_limit(average);
    }

    void resize(i32 group_num, bool clear = true) {
        init(group_num, clear);
        group_max_ = group_num;
        get_basic_info();
    }

    Group const & get_groups(int index) const {
        assert(index >= 0 && index < (int)groups.size());
        return groups[index];
    }

    void copy_from(Answer const & src) {
        this->group_max_ = src.group_max_;
        this->num_length_ = src.num_length_;
        this->num_used_ = src.num_used_;

        this->groups.clear();
        detail::copy_container(this->groups, src.groups);
        assert(this->group_max_ == src.groups.size());

        this->number_list.clear();
        copy_numbers(src.number_list);
        assert(this->num_length_ == this->number_list.size());
        get_basic_info();
    }

    bool move_num_to_last(i32 index, i32 remain_length) {
        assert(remain_length > 0 && remain_length <= (i32)number_list.size());
        assert(index >= 0 && index <= (i32)number_list.size());
        if (remain_length > 0) {
            if (index != remain_length - 1) {
                std::swap(number_list[index], number_list[remain_length - 1]);
            }
        }
        return (remain_length > 0);
    }

    bool fill_pair_numbers(i32 group_index, i32 index1, i32 index2) {
        assert(group_index >= 0 && group_index < (i32)groups.size());
        assert(index1 >= 0 && index1 < (i32)number_list.size());
        assert(index2 >= 0 && index2 < (i32)number_list.size());
        if (group_index >= 0 && group_index < (i32)groups.size()) {
            if (index1 != index2) {
                u32 value1, value2;
                value1 = number_list[index1];
                value2 = number_list[index2];

                if (groups[group_index].sum + value1 + value2 <= sum_limit_) {
                    NumberInfo number_info;
                    number_info.value = value1;
                    groups[group_index].numbers.push_back(number_info);
                    groups[group_index].sum += value1;
                    groups[group_index].length++;
                    num_used_++;

                    number_info.value = value2;
                    groups[group_index].numbers.push_back(number_info);
                    groups[group_index].sum += value2;
                    groups[group_index].length++;
                    num_used_++;
                    return true;
                }
            }
        }
        return false;
    }

    bool fill_pair_numbers(i32 group_index, i32 index) {
        assert(group_index >= 0 && group_index < (i32)groups.size());
        assert(index >= 0 && index < (i32)number_list.size());
        if (group_index >= 0 && group_index < (i32)groups.size()) {
            u32 value = number_list[index];
            if (groups[group_index].sum + value <= sum_limit_) {
                NumberInfo number_info;
                number_info.value = value;
                groups[group_index].numbers.push_back(number_info);
                groups[group_index].sum += value;
                groups[group_index].length++;
                num_used_++;
                return true;
            }
        }
        return false;
    }

    void fill_number(i32 group_idx, u32 value) {
        assert(group_idx >= 0 && group_idx < (i32)groups.size());
        NumberInfo number_info;
        number_info.value = value;
        groups[group_idx].numbers.push_back(number_info);
        groups[group_idx].sum += value;
        groups[group_idx].length++;
        num_used_++;
    }

    void unfill_number(i32 group_idx, u32 fill_numbers) {
        assert(group_idx >= 0 && group_idx < (i32)groups.size());
        for (u32 i = 0; i < fill_numbers; ++i) {
            NumberInfo & number_info = groups[group_idx].numbers.back();
            u32 value = number_info.value;
            groups[group_idx].numbers.pop_back();
            assert(groups[group_idx].sum >= value);
            groups[group_idx].sum -= value;
            groups[group_idx].length--;
            num_used_--;
            assert(num_used_ >= 0);
        }
    }

    bool fill_number(i32 group_idx, i32 index, i32 remain_length, u32 sum_limit) {
        assert(group_idx >= 0 && group_idx < (i32)groups.size());
        assert(remain_length >= 1 && remain_length <= (i32)number_list.size());
        assert(index >= 0 && index < (i32)number_list.size());
        if (group_idx >= 0 && group_idx < (i32)groups.size() && remain_length >= 1) {
            assert(index < remain_length);
            u32 value = number_list[index];
            if (groups[group_idx].sum + value <= sum_limit) {
                NumberInfo number_info;
                number_info.value = value;
                groups[group_idx].numbers.push_back(number_info);
                groups[group_idx].sum += value;
                groups[group_idx].length++;
                move_num_to_last(index, remain_length);
                num_used_++;
                sort_number_list(this->remain_numbers());
                return true;
            }
        }
        return false;
    }

    bool unfill_number(i32 group_idx, i32 index, i32 remain_length) {
        assert(group_idx >= 0 && group_idx < (i32)groups.size());
        assert(remain_length >= 1 && remain_length <= (i32)number_list.size());
        assert(index >= 0 && index < (i32)number_list.size());
        if (group_idx >= 0 && group_idx < (i32)groups.size() && remain_length >= 1) {
            u32 value = number_list[remain_length - 1];
            NumberInfo & number_info = groups[group_idx].numbers.back();
            assert(value == number_info.value);
            groups[group_idx].numbers.pop_back();
            assert(groups[group_idx].sum >= value);
            groups[group_idx].sum -= value;
            groups[group_idx].length--;
            //move_num_to_last(index, remain_length);
            num_used_--;
            sort_number_list(this->remain_numbers());
            return true;
        }
        return false;
    }

    void sort_numbers(bool is_asc = false, i32 length = -1) {
        // Sort per group number list, asc or desc order.
        for (size_t g = 0; g < groups.size(); ++g) {
            std::vector<NumberInfo> & numbers = groups[g].numbers;
            if (is_asc) {
                for (i32 i = 0; i < (i32)numbers.size() - 1; ++i) {
                    for (i32 j = i + 1; j < (i32)numbers.size(); ++j) {
                        if (numbers[i].value > numbers[j].value) {
                            std::swap(numbers[i], numbers[j]);
                        }
                    }
                }
            }
            else {
                for (i32 i = 0; i < (i32)numbers.size() - 1; ++i) {
                    for (i32 j = i + 1; j < (i32)numbers.size(); ++j) {
                        if (numbers[i].value < numbers[j].value) {
                            std::swap(numbers[i], numbers[j]);
                        }
                    }
                }
            }
        }

        // Sort number list, desc order.
        sort_number_list(length);
    }

    // Sort number list, desc order.
    void sort_number_list(i32 length = -1) {
        if (length == -1)
            length = (i32)number_list.size();
        for (i32 i = 0; i < length - 1; ++i) {
            for (i32 j = i + 1; j < length; ++j) {
                if (number_list[i] < number_list[j]) {
                    std::swap(number_list[i], number_list[j]);
                }
            }
        }
    }

private:
    void init(i32 group_num, bool clear) {
        if (clear) {
            groups.clear();
        }
        if (group_num > 0) {
            groups.resize(group_num);
        }

        i32 capacity = group_num * 3;
        for (i32 i = 0; i < group_num; ++i) {
            if (clear) {
                groups[i].numbers.clear();
            }
            if (capacity > 0)
                groups[i].numbers.reserve(capacity);
            groups[i].sum = 0;
            groups[i].length = 0;
            groups[i].minimum = 2147483647;
            groups[i].maximun = 0;
        }
    }

    u32 get_sum_limit(u32 average) {
        return (u32)(average * kSumLimitCoffe);
    }

    void get_basic_info() {
        u32 sum = 0;
        for (size_t i = 0; i < number_list.size(); ++i) {
            sum += number_list[i];
        }
        sum_ = sum;
        average_ = sum / group_max_;
        remain_  = sum % group_max_;
        sum_limit_ = get_sum_limit(average_);
    }
};

const double Answer::kSumLimitCoffe = GROUP_SUM_LIMIT_COFFE;

namespace detail {

void display_numbers(std::vector<u32> const & numbers, i32 columns)
{
    u32 length = (u32)numbers.size();
    printf("Numbers[%u] = {\n", length);
    for (u32 i = 0; i < length; ++i) {
        if (((i32)i % columns) == 0)
            printf("    ");
        if (i < length - 1)
            printf("%4d, ", numbers[i]);
        else
            printf("%4d", numbers[i]);
        if (((i32)i % columns) == (columns - 1))
            printf("\n");
    }
    printf("}\n\n");
}

void display_answer_detail(Answer const & answer, u32 sum, i32 length, i32 groups)
{
    double average = (double)sum / groups;
    double std_diff = 0.0;
    i32 avg_diff = 0, n_average = (i32)sum / groups;
    u32 remain = sum % (u32)groups;
    std::vector<u32> group_sums;

    if (groups > 0)
        group_sums.resize(groups);

    i32 max_group_len = 0;
    for (i32 g = 0; g < groups; ++g) {
        if (answer.groups[g].length > max_group_len) {
            max_group_len = answer.groups[g].length;
        }
    }

    printf("Answer: (Numbers = %d)\n\n", answer.used_numbers());

    // Display sum of per group.
    printf("  |");
    for (i32 g = 0; g < groups; ++g) {
        printf(" %4u |", answer.groups[g].sum);
    }
    printf("\n");

    // Display separator.
    printf("  -");
    for (i32 g = 0; g < groups; ++g) {
        printf("-------");
    }
    printf("\n");

    // Display numbers of per group.
    for (i32 index = max_group_len - 1; index >= 0; --index) {
        printf("  |");
        for (i32 g = 0; g < groups; ++g) {
            i32 group_length = (i32)answer.groups[g].numbers.size();
            if (index < group_length) {
                u32 number = answer.groups[g].numbers[index].value;
                printf(" %4u |", number);
                group_sums[g] += number;
            }
            else {
                printf("      |");
            }
        }
        printf("   %d\n", index + 1);
    }

    // Display separator.
    printf("  -");
    for (i32 g = 0; g < groups; ++g) {
        printf("-------");
    }
    printf("\n");

    // Display index of per group.
    printf("  |");
    for (i32 g = 0; g < groups; ++g) {
        printf("  %2d  |", g + 1);
    }
    printf("\n\n");

    // Calc standard diff and average diff.
    std_diff = 0.0f;
    avg_diff = 0;
    for (i32 g = 0; g < groups; ++g) {
        double diff = (double)group_sums[g] - average;
        std_diff += diff * diff;
        avg_diff += group_sums[g] - n_average;
    }

    // Display separator.
    printf("---");
    for (i32 g = 0; g < groups; ++g) {
        printf("-------");
    }
    printf("----\n");

    printf(" total sum:     %9u  average:       %9d  remain:       %9u\n", sum, n_average, remain); 
    printf(" group average: %9.3f  standard diff: %9.3f  average diff: %9.3f\n\n",
           average, std_diff / groups, (double)avg_diff / groups);
}

void display_answer_detail_old(Answer const & answer, u32 sum, i32 length, i32 groups)
{
    double average = (double)sum / groups;
    double std_diff = 0.0;
    i32 avg_diff = 0, n_average = (i32)sum / groups;

    printf("Answer[%d] = {\n", answer.used_numbers());
    for (i32 g = 0; g < groups; ++g) {
        printf("    group[%2d]: { ", g + 1);
        u32 sum = 0;
        size_t group_length = answer.groups[g].numbers.size();
        if (group_length != 0) {
            for (size_t i = 0; i < group_length; ++i) {
                u32 value = answer.groups[g].numbers[i].value;
                if (i < group_length - 1)
                    printf("%4d, ", value);
                else
                    printf("%4d", value);
                sum += value;
            }
        }
        else {
            printf(" <Empty> ");
        }
        double diff = (double)sum - average;
        std_diff += diff * diff;
        avg_diff += sum - n_average;
        printf(" }, sum = %u, len = %d.\n", sum, answer.groups[g].length);
    }
    printf("}\n\n");

    printf("group average: %0.3f, standard diff: %0.3f, average diff: %0.3f .\n\n",
           average, std_diff / groups, (double)avg_diff / groups);
}

} // namespace detail

class NumberGroup {
private:
    typedef RandomGenerator<u32> RandomGen;

    i32 group_max_, num_length_;
    u32 min_num_, max_num_;
    u32 sum_, average_, remain_;
    std::vector<u32> numbers_;
    std::vector<u32> sorted_list_;
    Answer answer_;

    const double kPrePickupCoffe = PRE_PICKUP_COFFE;

public:
    NumberGroup(i32 length, i32 group_num, u32 min_num, u32 max_num)
        : group_max_(group_num), num_length_(length), min_num_(min_num), max_num_(max_num),
          sum_(0), average_(0), remain_(0) {
        // Generate randomize numbers.
        generate_numbers(length, min_num, max_num);
    };
    ~NumberGroup() {}

    i32 group_max() const { return group_max_; }
    i32 num_length() const { return num_length_; }
    std::vector<u32> const & numbers() const { return numbers_; }

    u32 sum() const { return sum_; }
    u32 average() const { return average_; }
    u32 remain() const { return remain_; }

    void display_numbers() {
        printf("The default number list:\n\n");
        detail::display_numbers(numbers_, DISPLAY_COLUMNS);
    }

    void display_sorted_numbers() {
        printf("The sorted number list:\n\n");
        detail::display_numbers(sorted_list_, DISPLAY_COLUMNS);

        printf("sum = %u, remain = %u, group_average = %0.3f, num_average = %0.3f .\n\n",
               sum_, remain_, (double)sum_ / group_max_, (double)sum_ / num_length_);
    }

    void display_answer_numbers() {
        printf("The answer number list:\n\n");
        detail::display_numbers(answer_.numbers(), DISPLAY_COLUMNS);
    }

    void display_answers_detail() {
        printf("kPrePickupCoffe = %0.6f\n", kPrePickupCoffe);
        printf("kSumLimitCoffe  = %0.6f\n\n", Answer::kSumLimitCoffe);

        detail::display_numbers(answer_.numbers(), DISPLAY_COLUMNS);
        detail::display_answer_detail(answer_, sum_, num_length_, group_max_);
    }

    void display_answers() {
        printf("The answers is:\n\n");
        display_answers_detail();
    }

    void display_no_answers() {
        printf("Did not find any answers!\n\n");
    }

    void get_basic_info() {
        u32 sum = 0;
        for (size_t i = 0; i < numbers_.size(); ++i) {
            sum += numbers_[i];
        }
        sum_ = sum;
        average_ = sum / group_max_;
        remain_  = sum % group_max_;

        double group_average = (double)sum / group_max_;
        double num_average = (double)sum / num_length_;
        printf("sum = %u, remain = %u, group_average = %0.3f, num_average = %0.3f .\n\n",
               sum_, remain_, group_average, num_average);
    }

    void sort_numbers(i32 length = -1) {
        detail::copy_container< std::vector<u32> >(sorted_list_, numbers_);

        if (length == -1)
            length = (i32)numbers_.size();
        for (i32 i = 0; i < length - 1; ++i) {
            for (i32 j = i + 1; j < length; ++j) {
                if (sorted_list_[i] < sorted_list_[j]) {
                    std::swap(sorted_list_[i], sorted_list_[j]);
                }
            }
        }
    }

    int balance_pickup_numbers(Answer & result) {
        i32 step = num_length_ / 2;
        i32 group_index = 0;

        i32 remain_length = (i32)num_length_;
        i32 front, back, loop;
        bool exit = false;
        front = num_length_ / 2 - 1;
        if (front < 0)
            front = 0;
        back = front + 1;
        for (i32 i = 0; i < step; ++i) {
            group_index = RandomGen::next_i32(group_max_);
            loop = 0;
            do {
                bool success = result.fill_pair_numbers(group_index, front, back);
                group_index++;
                group_index %= group_max_;
                loop++;
                if (success || (loop > (i32)group_max_)) {
                    remain_length -= 2;
                    if (remain_length < 0)
                        exit = true;
                    front--;
                    back++;
                    if (front < 0 || back >= (i32)num_length_)
                        exit = true;
                    break;
                }
            } while (1);
            if (exit)
                break;
        }

        if (front < 0 && back < (i32)num_length_) {
            result.fill_pair_numbers(group_index, back);
        }
        else if (front >= 0 && back >= (i32)num_length_) {
            result.fill_pair_numbers(group_index, front);
        }

        return 1;
    }

    int random_pickup_numbers(Answer & result) {
        i32 remain_length = num_length_;
        i32 pickup_nums = (i32)(num_length_ * kPrePickupCoffe);
        while (remain_length > (num_length_ - pickup_nums)) {
            i32 group_index = RandomGen::next_i32(group_max_);
            i32 rand_index = RandomGen::next_i32(remain_length);
            if (result.fill_number(group_index, rand_index, remain_length, result.sum_limit())) {
                remain_length--;
            }
        }
        return remain_length;
    }

    void display_a_fill_list(const Answer & answer, i32 groud_idx,
                            const std::vector<u32> & fill_list, int depth) {
        printf("groud_idx = %d, sum = %u\n\n", groud_idx, answer.groups[groud_idx].sum);
        printf("[%d] = \" ", groud_idx);
        for (i32 i = 0; i < answer.groups[groud_idx].length; ++i) {
            if (i < answer.groups[groud_idx].length - 1)
                printf("%u, ", answer.groups[groud_idx].numbers[i].value);
            else
                printf("%u", answer.groups[groud_idx].numbers[i].value);
        }
        printf(" \"\n\n");

        printf("depth = %d, fill list is: [ ", depth);
        for (u32 i = 0; i < fill_list.size(); ++i) {
            if (i < fill_list.size() - 1)
                printf("%u, ", fill_list[i]);
            else
                printf("%u", fill_list[i]);
        }
        printf(" ]\n\n");
    }

    bool verify_last_group_sum(const Answer & answer, i32 groud_idx,
                               const std::vector<u32> & target_sums) {
        assert(groud_idx == (this->group_max() - 1));
        u32 group_sum = answer.groups[groud_idx].sum;
        u32 target_sum = target_sums[groud_idx];
        u32 sum = 0;
        for (int i = 0; i < answer.remain_numbers(); ++i) {
            sum += answer.number_list[i];
        }
        return (sum == group_sum);
    }

    void fill_last_group(Answer & answer, i32 groud_idx) {
        for (int i = 0; i < answer.remain_numbers(); ++i) {
            u32 value = answer.number_list[i];
            answer.fill_number(groud_idx, value);
        }        
    }

    void unfill_last_group(Answer & answer, i32 groud_idx, u32 fill_numbers) {
        assert(fill_numbers > 0);
        answer.unfill_number(groud_idx, fill_numbers);
    }

    int fill_one_group_impl(i32 depth, Answer & answer, i32 groud_idx,
                            std::vector<u32> & fill_list,
                            const std::vector<u32> & target_sums, u32 rest_sum) {
        int results = 0;
        u32 group_sum = answer.groups[groud_idx].sum;
        u32 target_sum = target_sums[groud_idx];
        assert(group_sum <= target_sum);
        assert(rest_sum > 0);
        assert(groud_idx <= this->group_max());
        i32 remain_numbers = answer.remain_numbers();
        for (i32 index = 0; index < remain_numbers; ++index) {
            u32 number = answer.number_list[index];
            if (number < rest_sum) {
                // Scan and skip the duplicate answers.
                bool is_duplicate = false;
                for (int i = (int)fill_list.size() - 1; i >= 0; --i) {
                    if (number > fill_list[i]) {
                        is_duplicate = true;
                        break;
                    }
                }
                if (is_duplicate) {
                    continue;
                }
                // Do fill number.
                if (answer.fill_number(groud_idx, index, remain_numbers, target_sum)) {
                    fill_list.push_back(number);
                    // Search next number in this group.
                    int next_results = fill_one_group_impl(depth + 1, answer, groud_idx,
                                                           fill_list, target_sums, rest_sum - number);
                    if (next_results > 0) {
                        results++;
                    }
                    // Undo fill number.
                    if (answer.unfill_number(groud_idx, index, remain_numbers)) {
                        fill_list.pop_back();
                    }
                }
            }
            else if (number == rest_sum) {
                // Scan and skip the duplicate answers.
                bool is_duplicate = false;
                for (int i = (int)fill_list.size() - 1; i >= 0; --i) {
                    if (number > fill_list[i]) {
                        is_duplicate = true;
                        break;
                    }
                }                
                if (is_duplicate) {
                    continue;
                }

                // Get a right group number list, continue to next group index.

                // Do fill number.
                if (answer.fill_number(groud_idx, index, remain_numbers, target_sum)) {
                    fill_list.push_back(number);
#if 0
                    if (groud_idx >= 8) {
                        display_a_fill_list(answer, groud_idx, fill_list, depth);
                    }
#endif
                    bool last_group_ok = false;
                    int last_fill_numbers = 0;
                    if (groud_idx == (this->group_max() - 2)) {
                        // Is last group.
                        last_group_ok = verify_last_group_sum(answer, groud_idx + 1, target_sums);
                        if (last_group_ok) {
                            last_fill_numbers = answer.remain_numbers();
                            if (last_fill_numbers > 0) {
                                // Do last group fill.
                                fill_last_group(answer, groud_idx + 1);
                            }
                        }
                    }
                    if (last_group_ok || (groud_idx >= (this->group_max() - 1))) {
                        g_result_cnt++;
                        // Get a right answer!
                        Answer copy_answer(answer);
                        copy_answer.sort_numbers(false, 0);
                        printf("The answers is:\n\n");
                        detail::display_numbers(copy_answer.numbers(), DISPLAY_COLUMNS);
                        detail::display_answer_detail(copy_answer, sum_, num_length_, group_max_);
                        printf("Press any key to continue ...");
                        jm_readkey();
                        printf("\n");
                        // Undo last group fill.
                        if (last_group_ok) {
                            if (last_fill_numbers > 0) {
                                unfill_last_group(answer, groud_idx + 1, last_fill_numbers);
                            }
                        }
                        // Undo fill number.
                        if (answer.unfill_number(groud_idx, index, remain_numbers)) {
                            fill_list.pop_back();
                        }
                        return 1;
                    }
                    else if (groud_idx < (this->group_max() - 1)) {
                        // Search next group numbers.
                        results = fill_one_group(answer, groud_idx + 1, target_sums);
                    }
                }
                // Undo fill number.
                if (answer.unfill_number(groud_idx, index, remain_numbers)) {
                    fill_list.pop_back();
                }
                return 1;
            }
        }
        return results;
    }

    bool fill_one_group(const Answer & answer, i32 groud_idx,
                        const std::vector<u32> & target_sums) {
        u32 group_sum = answer.groups[groud_idx].sum;
        u32 target_sum = target_sums[groud_idx];
        u32 rest_sum = (target_sum >= group_sum) ? (target_sum - group_sum) : (target_sum - target_sum);

        Answer next_group(answer);
        std::vector<u32> fill_list;

        // Sorted remain numbers first.
        next_group.sort_numbers(false, next_group.remain_numbers());
#if 0
        if (groud_idx > 8) {
            printf("The answers is:\n\n");
            detail::display_numbers(next_group.numbers(), DISPLAY_COLUMNS);
            detail::display_answer_detail(next_group, sum_, num_length_, group_max_);
            printf("Press any key to continue ...");
            jm_readkey();
            printf("\n");
        }
#endif
        int results = fill_one_group_impl(0, next_group, groud_idx, fill_list, target_sums, rest_sum);
        return (results > 0);
    }

    int try_to_slove() {
        Answer answer_slove(answer_);
        u32 remain_cnt = 0;
        i32 group_idx = 0;
        std::vector<u32> target_sums;

        if (this->group_max() > 0)
            target_sums.resize(this->group_max());
        while (group_idx < this->group_max()) {
            if (remain_cnt < answer_slove.remain()) {
                target_sums[group_idx] = this->average() + 1;
                remain_cnt++;
            }
            else {
                target_sums[group_idx] = this->average();
            }
            group_idx++;
        }

        g_result_cnt = 0;
        bool success = fill_one_group(answer_slove, 0, target_sums);
        if (success) {
            printf("g_result_cnt = %d\n\n", g_result_cnt);
            jm_readkey();
            printf("\n");
        }
        return g_result_cnt;
    }

    void prepare() {
        get_basic_info();

        sort_numbers();
        display_sorted_numbers();
    }

    int pre_pickup_numbers() {
        answer_.resize(group_max_);
        answer_.copy_numbers(sorted_list_);

        //RandomSeed::force_reinit(2017);
        //RandomSeed::init();

        Answer answer(group_max_, num_length_, sorted_list_);
        int used_numbers = random_pickup_numbers(answer);
        //int used_numbers = balance_pickup_numbers(answer);
        if (used_numbers > 0) {
            answer_.copy_from(answer);
        }
        answer_.sort_numbers(false, answer_.remain_numbers());

        display_answers();
        printf("Press any key to continue ...");
        jm_readkey();
        printf("\n");

        return used_numbers;
    }

    int slove() {
        prepare();

        int answer_num;
        do {
            pre_pickup_numbers();
            answer_num = try_to_slove();
        } while (answer_num <= 0);
        return answer_num;
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
    // Force init fixed randomize seed.
    //RandomSeed::force_reinit(2017);
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

#if defined(WIN32) || defined(_WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS)
    ::system("pause");
#endif
    return 0;
}
