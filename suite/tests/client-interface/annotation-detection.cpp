
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include "dr_annotations.h"
#include "test_annotation_arguments.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
# define INLINE __inline
#else
# define INLINE inline
# ifdef __LP64__
#  define uintptr_t unsigned long long
# else
#  define uintptr_t unsigned long
# endif
#endif

class Shape
{
    public:
        virtual double get_area() = 0;
        virtual unsigned int get_vertex_count() = 0;
};

class Square : public Shape
{
    public:
        Square(double side_length);
        double get_side_length();
        double get_area();
        unsigned int get_vertex_count();

    private:
        double side_length;
};


class Triangle : public Shape
{
    public:
        Triangle(double a, double b, double c);
        double get_a();
        double get_b();
        double get_c();
        double get_area();
        unsigned int three();
        void set_lengths(double a, double b, double c);
        unsigned int get_vertex_count();

    private:
        double lengths[3];

        double calculate_s();
};

class Fail : public std::runtime_error
{
    public:
        Fail(int error_code);
        int get_error_code() const;

    private:
        int error_code;
};

Square::Square(double side_length) : side_length(side_length)
{
}

INLINE double Square::get_side_length()
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, (unsigned int) side_length, {
        printf("Native two-args in Square::get_side_length()\n");
    });

    return side_length;
}

double Square::get_area()
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__,
                             (unsigned int) (get_side_length() * get_side_length()), {
        printf("Native two-args in Square::get_area()\n");
    });

    return get_side_length() * get_side_length();
}

#pragma auto_inline(off)
unsigned int Square::get_vertex_count()
{
    return 4;
}
#pragma auto_inline(on)

Triangle::Triangle(double a, double b, double c)
{
    set_lengths(a, b, c);

    TEST_ANNOTATION_THREE_ARGS(__LINE__, (unsigned int) b,
                               (unsigned int) get_area());
}

unsigned int Triangle::three()
{
    return TEST_ANNOTATION_THREE_ARGS((unsigned int) get_a(), (unsigned int) get_b(),
                                      (unsigned int) get_c());
}

INLINE void Triangle::set_lengths(double a, double b, double c)
{
    lengths[0] = a;

    TEST_ANNOTATION_TWO_ARGS(__LINE__, (unsigned int) b, {
        printf("Native two-args in Square::set_lengths()\n");
    });

    lengths[1] = b;
    lengths[2] = c;

    TEST_ANNOTATION_THREE_ARGS(__LINE__, (unsigned int) b,
                               (unsigned int) get_area());
}

INLINE double Triangle::get_a()
{
    return lengths[0];
}

INLINE double Triangle::get_b()
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, (unsigned int) get_c(), {
        printf("Native two-args in Triangle::get_b()\n");
    });

    return lengths[1];
}

INLINE double Triangle::get_c()
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, (unsigned int) get_a(), {
        printf("Native two-args in Triangle::get_c()\n");
    });
    TEST_ANNOTATION_THREE_ARGS(__LINE__, 0x77, 0x7890);

    return lengths[2];
}

INLINE double Triangle::get_area()
{
    double s = calculate_s();
    s *= (s - get_a());
    printf("get_area(): s with a: %f\n", s);
    s *= (s - get_b());
    printf("get_area(): s with b: %f\n", s);
    s *= (s - get_c());
    printf("get_area(): s with c: %f\n", s);
    return s;
}

#pragma auto_inline(off)
unsigned int Triangle::get_vertex_count()
{
    return 3;
}
#pragma auto_inline(on)

double Triangle::calculate_s()
{
    return (lengths[0] + lengths[1] + lengths[2]) / 2.0;
}

Fail::Fail(int error_code) : runtime_error("foo"), error_code(error_code)
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, error_code, {
        printf("Native two-args in Fail::Fail()\n");
    });
}

int Fail::get_error_code() const
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, error_code, {
        printf("Native two-args in Fail::get_error_code()\n");
    });

    return error_code;
}

static void
annotation_wrapper(int a, int b, int c, int d)
{
    if (DYNAMORIO_ANNOTATE_RUNNING_ON_DYNAMORIO())
        TEST_ANNOTATION_EIGHT_ARGS(a, b, c, d, a, b, c, d);
}

static int
power(int x, unsigned int exp)
{
    if (x == 0)
        return 0;
    if (exp == 0)
        return 1;

    unsigned int i;
    int base = x;
    for (i = 1; i < exp; i++)
        x *= base;
    return x;
}

static INLINE int
two()
{
    TEST_ANNOTATION_TWO_ARGS(__LINE__, 5, {
        printf("Native two args: %d, %d\n", __LINE__, 5);
    });
    return 2;
}

static INLINE int
three(unsigned int a, unsigned int b)
{
    return TEST_ANNOTATION_THREE_ARGS(__LINE__, a, b);
}

static void
colocated_annotation_test()
{
    TEST_ANNOTATION_EIGHT_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8); TEST_ANNOTATION_NINE_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9);
}

// C control flow: ?, switch, if/else, goto, return <value>, setjmp/longjmp,
// C++ control flow: exceptions, vtable?

int main(void)
{
    unsigned int i, j;

    Shape *shape;
    Triangle *t = new Triangle(4.3, 5.2, 6.1);
    Square *s = new Square(7.0);

    //if (1) return 0;

    shape = t;
    printf("Triangle [%f x %f x %f] area: %f (%d)\n", t->get_a(), t->get_b(), t->get_c(),
           t->get_area(), three((unsigned int) shape->get_vertex_count(), t->three()));

    shape = s;
    printf("Square [%f x %f] area: %f (%d)\n", s->get_side_length(), s->get_side_length(),
           shape->get_area(),
           three((unsigned int) shape->get_vertex_count(),
                 TEST_ANNOTATION_THREE_ARGS(t->three(), t->three(), t->three())));


    try {
        TEST_ANNOTATION_NINE_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9);
        throw (Fail(TEST_ANNOTATION_THREE_ARGS((unsigned int) t->get_b(),
                                                   (unsigned int) shape->get_area(), 4)));
        TEST_ANNOTATION_TWO_ARGS(two(), 4, { printf("Native line %d\n", __LINE__); });
    } catch (const Fail& fail) {
        TEST_ANNOTATION_TWO_ARGS(1, two(), { printf("Native line %d\n", __LINE__); });
        printf("Fail! %d\n", fail.get_error_code());
    }

    TEST_ANNOTATION_TWO_ARGS(two(), 4, { printf("Native line %d\n", __LINE__); });
    printf("three args #0: %d\n", TEST_ANNOTATION_THREE_ARGS(1, 2, 3));
    printf("three args #1: %d\n", TEST_ANNOTATION_THREE_ARGS(three(9, 8), two(), 1));
    printf("three args #2: %d\n", TEST_ANNOTATION_THREE_ARGS(two(), 4, three(2, 3)));

    colocated_annotation_test();

    j = ((unsigned int) shape->get_area()) % 11;
    for (i = 0; i < 10; i++) {
        switch ((i + j) % 10) {
            case 0:
                TEST_ANNOTATION_NINE_ARGS(
                    power(2, power(i, 3) % 9), power(3, 4), power(i, j), power(2, i),
                    power(two(), 3), power(3, 4), DYNAMORIO_ANNOTATE_RUNNING_ON_DYNAMORIO(),
                    power(i, j), power(DYNAMORIO_ANNOTATE_RUNNING_ON_DYNAMORIO(), i));
            case 1:
                TEST_ANNOTATION_EIGHT_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8);
            case 2:
case2:
            case 3:
            case 4:
                TEST_ANNOTATION_NINE_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9);
                TEST_ANNOTATION_EIGHT_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8);
                break;
            case 5:
                TEST_ANNOTATION_NINE_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9);
                annotation_wrapper(i, j, i + j, i * j);
                TEST_ANNOTATION_TEN_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9, 10);
                break;
            case 6:
                TEST_ANNOTATION_EIGHT_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8);
            case 7: {
                unsigned int a = 0, b = 0;
                TEST_ANNOTATION_TEN_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9, 10);
                TEST_ANNOTATION_TEN_ARGS(__LINE__, 2, 3, power(4, b), 5, 6, 7, 8, 9, 10);
                a = b;
                if (b > 0)
                    goto case2;
            }
            case 8:
            case 9:
            default:
                TEST_ANNOTATION_EIGHT_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8);
                TEST_ANNOTATION_NINE_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9);
                TEST_ANNOTATION_TEN_ARGS(__LINE__, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        }
    }
}

#ifdef __cplusplus
}
#endif
