/*

 Copyright (c) 2023 Oliver Lau <oliver.lau@gmail.com>

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the “Software”), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef __GETOPT_HPP__
#define __GETOPT_HPP__

#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace argparser
{

    class argument_required_exception : public std::exception
    {
        std::string exception_message;

    public:
        explicit argument_required_exception(std::string const &option_name)
        {
            exception_message = "Option `" + option_name + "` requires an argument, but none is given.";
        }
        const char *what() const throw()
        {
            return exception_message.c_str();
        }
    };

    class argparser
    {
    public:
        enum arg_type_t
        {
            no_argument,
            required_argument,
            optional_argument
        };
        using callback_t = std::function<void(std::string const &)>;
        struct arg_options
        {
            callback_t handler;
            arg_type_t arg_type;
        };

        argparser() = delete;
        argparser(int argc, char *argv[])
        {
            const std::size_t n = static_cast<std::size_t>(argc);
            args_.reserve(n);
            for (std::size_t i = 1; i < n; ++i)
            {
                args_.push_back(argv[i]);
            }
        }

        argparser &reg(std::vector<std::string> const &options, arg_type_t arg_type, callback_t handler)
        {
            for (auto const &opt : options)
            {
                options_.emplace(std::make_pair(
                    opt,
                    arg_options{handler, arg_type}));
            }
            return *this;
        }

        argparser &pos(callback_t handler)
        {
            positionals_.push_back(handler);
            return *this;
        }

        void operator()()
        {
            current_positional_ = positionals_.begin();
            auto arg = args_.begin();
            while (arg != args_.end())
            {
                if (options_.find(*arg) != options_.end())
                {
                    arg_options const &opt = options_.at(*arg);
                    switch (opt.arg_type)
                    {
                    case no_argument:
                        opt.handler(std::string());
                        break;
                    case required_argument:
                    {
                        if (std::next(arg) != args_.end())
                        {
                            opt.handler(*(++arg));
                        }
                        else
                        {
                            throw argument_required_exception(*arg);
                        }
                        break;
                    }
                    case optional_argument:
                    {
                        if (std::next(arg) != args_.end() && options_.find(*std::next(arg)) == options_.end())
                        {
                            opt.handler(*(++arg));
                        }
                        else
                        {
                            opt.handler(std::string());
                        }
                        break;
                    }
                    }
                }
                else
                {
                    if (current_positional_ != positionals_.end())
                    {
                        (*current_positional_)(*arg);
                        ++current_positional_;
                    }
                }
                ++arg;
            }
        }

    private:
        std::vector<std::string> args_;
        std::vector<callback_t> positionals_;
        std::vector<callback_t>::const_iterator current_positional_;
        std::unordered_map<std::string, arg_options> options_;
    };

}

#endif // __GETOPT_HPP__
