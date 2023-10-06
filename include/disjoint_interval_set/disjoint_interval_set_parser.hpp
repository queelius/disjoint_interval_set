#pragma once

#include <sstream>
#include <string_view>
#include <string>
#include <algorithm>
#include <regex>

using std::string;
using std::sregex_iterator;
using std::numeric_limits;
using std::stringstream;
using std::ostream;
using std::regex;
using std::string_view;

namespace set::interval
{
    /**
     * This is a simpler parser for interval sets.
     * It maps a string-encoded list of intervals to a set of intervals.
     */
    template <typename Set>
    void make_interval_set(string_view s, Set & is)
    {
        using interval_type = typename Set::interval_type;

        const static string N = "[-+]?([0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?|inf(inity)?)";
        const static string L = "(\\(|\\[)";
        const static string R = "(\\)|\\])";
        const static string S = " *";
        const static string INF = "(+|-)?inf(inity)?";
        const static regex r("(" + N + "|" + L + S + N + S + "," + S + N + S + R + ")");

        for (auto i = sregex_iterator(s.begin(), s.end(), r); i != sregex_iterator(); ++i)
        {
            T value[2];
            bool open[2];
            T factor;
            stringstream ss(i->str());
            auto skip = [&ss]() -> void
            {
                while (isspace(ss.peek()))
                    ss.get();
            };
            char c;

            if (ss.peek() == '[' || ss.peek() == '(')
            {
                skip();
                open[0] = (ss.get() == '(');

                for (int j = 0; j < 2; ++j)
                {
                    factor = static_cast<T>(1);
                    skip();
                    if (ss.peek() == '+' || ss.peek() == '-')
                        factor = (ss.get() == '-' ? -1 : 1);

                    if (ss.peek() == 'i')
                    {
                        value[j] = numeric_limits<T>::infinity();
                        while (isalpha(ss.peek()))
                            ss.get();
                    }
                    else
                        ss >> value[j];
                    value[j] *= factor;

                    if (j == 0)
                        ss >> c; // capture comma
                }

                ss >> c;
                open[1] = c == ')';
                is.push_back(interval_type(value[0], value[1], open[0], open[1]));
            }
            else
            {
                ss >> value[0];
                is.push_back(interval_type(value[0],value[0],false,false));
            }
        }
    }
}
