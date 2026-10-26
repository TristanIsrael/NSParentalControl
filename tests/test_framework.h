/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdio>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace tf {

    struct Case {
        const char* name;
        void (*fn)();
    };

    inline std::vector<Case>& registry() {
        static std::vector<Case> r;
        return r;
    }

    struct Registrar {
        Registrar(const char* name, void (*fn)()) {
            registry().push_back({name, fn});
        }
    };

    inline int& failures() {
        static int f = 0;
        return f;
    }

    inline int& checks() {
        static int c = 0;
        return c;
    }

    template <typename A, typename B>
    void expectEq(const A& a, const B& b, const char* exprA, const char* exprB, const char* file, int line) {
        ++checks();
        if(!(a == b)) {
            ++failures();
            std::ostringstream oa, ob;
            oa << a;
            ob << b;
            std::printf("  FAIL %s:%d: %s == %s  (%s != %s)\n", file, line, exprA, exprB, oa.str().c_str(), ob.str().c_str());
        }
    }

    inline void expectTrue(bool v, const char* expr, const char* file, int line) {
        ++checks();
        if(!v) {
            ++failures();
            std::printf("  FAIL %s:%d: %s\n", file, line, expr);
        }
    }

    int runAll() {
        for(const auto& c : registry()) {
            std::printf("[ RUN  ] %s\n", c.name);
            const int before = failures();
            c.fn();
            if(failures() == before) {
                std::printf("[  OK  ] %s\n", c.name);
            } else {
                std::printf("[ FAIL ] %s\n", c.name);
            }
        }
        std::printf("\n%d check(s), %d failure(s), %d test(s)\n", checks(), failures(), static_cast<int>(registry().size()));
        return failures() == 0 ? 0 : 1;
    }

}

#define TEST(name) \
    static void tf_test_##name(); \
    static ::tf::Registrar tf_reg_##name(#name, &tf_test_##name); \
    static void tf_test_##name()

#define CHECK_TRUE(expr) ::tf::expectTrue((expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(a, b) ::tf::expectEq((a), (b), #a, #b, __FILE__, __LINE__)
