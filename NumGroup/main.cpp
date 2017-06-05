
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

#define noif(statement) \
    do { \
        statement; \
    } while (0);

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
        u32 sum, length;
        u32 minimum, maximun;
    };

    u32 group_num_;
    u32 length_;
    u32 total_;
    u32 sum_;
    u32 average_;
    u32 remain_;
    u32 sum_limit_;

public:
    std::vector<Group> groups;
    std::vector<u32> number_list;

    const double kSumLimitCoffe = 0.6f;

public:
    Answer() : group_num_(0), length_(0), total_(0),
               sum_(0), average_(0), remain_(0), sum_limit_(0) {
    }

    Answer(u32 n_groups, u32 n_length)
        : group_num_(n_groups), length_(n_length), total_(0),
          sum_(0), average_(0), remain_(0), sum_limit_(0) {
        init(n_groups, false);
        get_basic_info();
    }

    Answer(u32 n_groups, u32 n_length, std::vector<u32> const & numbers)
        : group_num_(n_groups), length_(n_length), total_(0),
          sum_(0), average_(0), remain_(0), sum_limit_(0) {
        init(n_groups, false);
        copy_numbers(numbers);
    }

    Answer(const Answer & src) : Answer() {
        copy_from(src);
    }

    ~Answer() {}

    u32 group_num() const { return group_num_; }
    u32 length() const { return length_; }
    std::vector<u32> const & numbers() const { return number_list; }

    u32 sum() const { return sum_; }
    u32 average() const { return average_; }
    u32 remain() const { return remain_; }

    u32 total_numbers() const { return total_; }

    void copy_numbers(std::vector<u32> const & numbers) {
        detail::copy_container< std::vector<u32> >(number_list, numbers);
        length_ = (u32)number_list.size();
        get_basic_info();
    }

    void set_basic_info(u32 sum, u32 average, u32 remain) {
        sum_ = sum;
        average_ = average;
        remain_ = remain;
        sum_limit_ = get_sum_limit(average);
    }

    void resize(u32 n_groups, bool clear = true) {
        init(n_groups, clear);
        group_num_ = n_groups;
    }

    Group const & get_groups(int index) const {
        assert(index >= 0 && index < (int)groups.size());
        return groups[index];
    }

    void copy_from(Answer const & src) {
        this->group_num_ = src.group_num_;
        this->length_ = src.length_;
        this->total_ = src.total_;

        this->groups.clear();
        detail::copy_container(this->groups, src.groups);
        assert(this->group_num_ == src.groups.size());

        this->number_list.clear();
        copy_numbers(src.number_list);        
        assert(this->length_ == this->number_list.size());
        get_basic_info();
    }

    bool move_num_to_last(i32 index, i32 remain_length) {
        assert(remain_length > 0 && remain_length <= (i32)number_list.size());
        assert(index >= 0 && index <= (i32)number_list.size());
        if (remain_length > 0) {
            if (index != remain_length - 1)
                std::swap(number_list[index], number_list[remain_length - 1]);
        }
        return (remain_length > 0);
    }

    bool pick_pair_numbers(u32 group_index, i32 index1, i32 index2) {
        assert(group_index >= 0 && group_index < groups.size());
        assert(index1 >= 0 && index1 < (i32)number_list.size());
        assert(index2 >= 0 && index2 < (i32)number_list.size());
        if (group_index >= 0 && group_index < groups.size()) {
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
                    total_++;

                    number_info.value = value2;
                    groups[group_index].numbers.push_back(number_info);
                    groups[group_index].sum += value2;
                    groups[group_index].length++;
                    total_++;
                    return true;
                }
            }
        }
        return false;
    }

    bool pick_pair_numbers(u32 group_index, i32 index) {
        assert(group_index >= 0 && group_index < groups.size());
        assert(index >= 0 && index < (i32)number_list.size());
        if (group_index >= 0 && group_index < groups.size()) {            
            u32 value = number_list[index];
            if (groups[group_index].sum + value <= sum_limit_) {
                NumberInfo number_info;
                number_info.value = value;
                groups[group_index].numbers.push_back(number_info);
                groups[group_index].sum += value;
                groups[group_index].length++;
                total_++;
                return true;
            }
        }
        return false;
    }

    bool pick_numbers(u32 group_index, i32 index, i32 remain_length) {
        assert(group_index >= 0 && group_index < groups.size());
        assert(remain_length >= 1 && remain_length <= (i32)number_list.size());
        assert(index >= 0 && index < (i32)number_list.size());
        if (group_index >= 0 && group_index < groups.size() && remain_length >= 1) {            
            u32 value = number_list[index];
            if (groups[group_index].sum + value <= sum_limit_) {
                NumberInfo number_info;
                number_info.value = value;
                groups[group_index].numbers.push_back(number_info);
                move_num_to_last(index, remain_length);
                groups[group_index].sum += value;
                groups[group_index].length++;
                total_++;
                return true;
            }
        }
        return false;
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
    void init(u32 n_group, bool clear) {
        if (clear) {
            groups.clear();
        }
        groups.resize(n_group);

        u32 capacity = n_group * 3;
        for (size_t i = 0; i < n_group; ++i) {
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

    u32 get_sum_limit(u32 average) {
        return (u32)(average * kSumLimitCoffe);
    }

    void get_basic_info() {
        u32 sum = 0;
        for (size_t i = 0; i < number_list.size(); ++i) {
            sum += number_list[i];
        }
        sum_ = sum;
        average_ = sum / group_num_;
        remain_ = sum % length_;
        sum_limit_ = get_sum_limit(average_);
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

    printf("Answer[%u] = {\n", answer.total_numbers());
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
        printf(" }, sum = %u, len = %u.\n", sum, answer.groups[g].length);
    }
    printf("}\n\n");

    printf("group average: %0.3f, standard diff: %0.3f, average diff: %0.3f .\n\n",
           average, std_diff / groups, (double)n_diff / groups);
}

} // namespace detail

class NumberGroup {
private:
    typedef RandomGenerator<u32> RandomGen;

    u32 group_num_, length_;
    u32 min_num_, max_num_;
    u32 sum_, average_, remain_;
    std::vector<u32> numbers_;
    std::vector<u32> sorted_list_;
    Answer answer_;

    const double kPrePickupCoffe = 0.6f;

public:
    NumberGroup(u32 groups, u32 length, u32 min_num, u32 max_num)
        : group_num_(groups), length_(length), min_num_(min_num), max_num_(max_num),
          sum_(0), average_(0), remain_(0) {
        // Generate randomize numbers.
        generate_numbers(length, min_num, max_num);
    };
    ~NumberGroup() {}

    u32 group_num() const { return group_num_; }
    u32 length() const { return length_; }
    std::vector<u32> const & numbers() const { return numbers_; }

    u32 sum() const { return sum_; }
    u32 average() const { return average_; }
    u32 remain() const { return remain_; }

    void display_numbers() {
        detail::display_numbers(numbers_, DISPLAY_COLUMNS);
    }

    void display_sorted_numbers() {
        detail::display_numbers(sorted_list_, DISPLAY_COLUMNS);
    }

    void display_answers_detail() {
        printf("kPrePickupCoffe = %0.6f\n\n", kPrePickupCoffe);
        detail::display_answer_detail(answer_, sum_, length_, group_num_);
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
        average_ = sum / group_num_;
        remain_ = sum % group_num_;

        double group_average = (double)sum / group_num_;
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

        i32 remain_length = (i32)length_;
        i32 front, back, loop;
        bool exit = false;
        front = length_ / 2 - 1;
        if (front < 0)
            front = 0;
        back = front + 1;
        for (u32 i = 0; i < step; ++i) {
            group_index = RandomGen::next(group_num_);
            loop = 0;
            do {
                bool success = result.pick_pair_numbers(group_index, front, back);
                group_index++;
                group_index %= group_num_;
                loop++;
                if (success || (loop > (i32)group_num_)) {
                    remain_length -= 2;
                    if (remain_length < 0)
                        exit = true;
                    front--;
                    back++;
                    if (front < 0 || back >= (i32)length_)
                        exit = true;
                    break;
                }
            } while (1);
            if (exit)
                break;
        }

        if (front < 0 && back < (i32)length_) {
            result.pick_pair_numbers(group_index, back);
        }
        else if (front >= 0 && back >= (i32)length_) {
            result.pick_pair_numbers(group_index, front);
        }
        
        return 1;
    }

    int random_pick_numbers(Answer & result) {
        i32 remain_length = (i32)length_;
        const i32 pick_nums = (const i32)(length_ * kPrePickupCoffe);
        while (remain_length > ((i32)length_ - pick_nums)) {
            u32 group_index = RandomGen::next(group_num_);
            i32 rand_index = RandomGen::next_i32(remain_length);
            if (result.pick_numbers(group_index, rand_index, remain_length)) {
                remain_length--;
            }
        }
        return remain_length;
    }

    void prepare() {
        get_basic_info();

        sort_numbers();
        display_sorted_numbers();
    }

    int pre_pickup_numbers() {
        answer_.resize(group_num_);
        answer_.copy_numbers(sorted_list_);

        //RandomSeed::force_reinit(2017);
        
        Answer answer(group_num_, length_, sorted_list_);
        int used_numbers = random_pick_numbers(answer);
        //int used_numbers = balance_pick_numbers(answer);
        if (used_numbers > 0) {
            answer_.copy_from(answer);
        }
        answer_.sort_numbers(false);
        return used_numbers;
    }

    bool fill_a_group(const Answer & result, u32 groud_idx, bool has_remain) {
        int i = result.groups[0].length;
        return true;
    }

    int try_to_slove() {
        Answer answer_slove(answer_);
        u32 remain_cnt = 0;
        u32 group_idx = 0;
        bool success = false;
        while (group_idx < group_num_) {
            if (answer_slove.remain() < remain_cnt) {
                success = fill_a_group(answer_slove, group_idx, true);
                if (success)
                    remain_cnt++;
            }
            else {
                success = fill_a_group(answer_slove, group_idx, false);
            }
            if (!success)
                break;
            group_idx++;
        }
        return (int)success;
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
    RandomSeed::force_reinit(2017);

    NumberGroup slover(GROUP_COUNT, NUMBER_COUNT, 5, 100);

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
