/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <math.h>
#include <flux/testing/TestSuite>
#include <flux/stdio>
#include <flux/Color>

using namespace flux;
using namespace flux::testing;

class GrayShadesOnPaper: public TestCase
{
    void run() {
        Color a = Color(0x00, 0x00, 0x00);
        Color b = Color(0xFF, 0xFF, 0xFF);
        for (int i = 0; i <= 0xFF; ++i) {
            Color::alpha(a) = i;
            Color c = Color::blend(a, b);
            Color d = Color(0xFF - i, 0xFF - i, 0xFF - i);
            if (c != d) fout("%%, %%\n") << c << d;
            FLUX_VERIFY(c == d);
        }
    }
};

class GrayShadesOnGlass: public TestCase
{
    void run()
    {
        Color a = Color(0x00, 0x00, 0x00);
        Color b = Color(0xFF, 0xFF, 0xFF, 0x80);
        for (int i = 0; i <= 0xFF; ++i) {
            Color::alpha(a) = i;
            Color c = Color::blend(a, b);
            Color d = GrayShadesOnGlass::blend(a, b);
            if (c != d) fout("%% over %% => %% (%%)\n") << a << b << c << d;
            FLUX_VERIFY(differLess(c, d, 1, 1));
        }
    }

    inline static int delta(int a, int b) { return a < b ? b - a : a - b; }

    static bool differLess(Color a, Color b, int maxColorDelta, int maxOpacityDelta) {
        return delta(Color::red(a), Color::red(b)) <= maxColorDelta &&
        delta(Color::green(a), Color::green(b)) <= maxColorDelta &&
        delta(Color::blue(a), Color::blue(b)) <= maxColorDelta &&
        delta(Color::alpha(a), Color::alpha(b)) <= maxOpacityDelta;
    }

    static double blendComponent(double alpha, double a, double beta, double b) {
        return
            (alpha * a + (1 - alpha) * beta * b)
            /
            (alpha + (1 - alpha) * beta);
    }

    static double blendAlpha(double alpha, double beta) {
        return alpha + (1 - alpha) * beta;
    }

    static Color blend(Color a, Color b) {
        double a_r   = Color::red(a)   / 255.;
        double a_g   = Color::green(a) / 255.;
        double a_b   = Color::blue(a)  / 255.;
        double alpha = Color::alpha(a) / 255.;
        double b_r   = Color::red(b)   / 255.;
        double b_g   = Color::green(b) / 255.;
        double b_b   = Color::blue(b)  / 255.;
        double beta  = Color::alpha(b) / 255.;
        return Color(
            ::round(blendComponent(alpha, a_r, beta, b_r) * 255),
            ::round(blendComponent(alpha, a_g, beta, b_g) * 255),
            ::round(blendComponent(alpha, a_b, beta, b_b) * 255),
            ::round(blendAlpha(alpha, beta) * 255)
        );
    }
};

int main(int argc, char **argv)
{
    // for (int x = 0; x < 256; ++x)
    //    fout("%%, %%\n") << x << 256 * x / 255 + (x % 255 > 127);

    FLUX_TESTSUITE_ADD(GrayShadesOnPaper);
    FLUX_TESTSUITE_ADD(GrayShadesOnGlass);

    return testSuite()->run(argc, argv);
}
