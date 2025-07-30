
#include <iostream>
#include <ranges>
#include <optional>

#include <qrcode/qrcode.h>
#include <qrcode/svg.h>

using namespace qrcode;
using namespace qrcode::qr;

namespace
{
    struct arguments
    {
        error_correction error_level = error_correction::level_L;
        std::optional<symbol_version> version = std::nullopt;
        std::optional<eci::assignment_number> eci_number;
        std::string_view message;
    };

    [[nodiscard]] auto print_designator(std::ostream& stream, arguments const& options)
    {
        if (!options.version)
        {
            stream << "requested type   (auto-detect)\n";     
            return;
        }

        auto const type = 
            symbol_designator{.version=*(options.version), .error_level=options.error_level};
        stream << "requested type   " << type << "\n";
    }

    [[nodiscard]] auto print_eci_number(std::ostream& stream, arguments const& options)
    {
        if (!options.eci_number)
        {
            stream << "eci number       (not set)\n";
            return;
        }
        stream << "eci number       " << options.eci_number->value << "\n";
    }

    template<class Message>
    [[nodiscard]] auto print_message(std::ostream& stream, Message&& message)
    {
        stream << "message          " << message << "\n";
    }

    [[nodiscard]] auto print_options(std::ostream& stream, arguments const& options)
    {
        print_designator(stream, options);
        print_eci_number(stream, options);
    }

    [[nodiscard]] auto print_usage(std::ostream& stream, std::string_view program_name)
    {
        stream
            << "usage: "
            << "echo \"Hello World\" | "
            << program_name
            << "<error correction level> [<symbol version> <eci_number>]\n";
    }

    [[nodiscard]] auto print_error(std::ostream& stream, error_code error)
    {
        switch(error)
        {
        case error_code::data_too_large: 
            stream << "data too large\n";
            break;
        case error_code::version_does_not_support_given_data: 
            stream << "version does not support given data\n";
            break;
        case error_code::version_and_error_level_not_supported: 
            stream << "version and error level not supported\n";
            break;
        }
    }

    [[nodiscard]] constexpr auto message_option(arguments options, char** current, char**)
        -> std::optional<std::pair<char**, arguments>>
    {
        options.message = std::string_view{*current};
        return std::pair{current, options};
    }

    [[nodiscard]] constexpr auto error_level_option(arguments options, char** current, char** next)
        -> std::optional<std::pair<char**, arguments>>
    {
        if (next == nullptr)
            return std::nullopt;

        auto const level = **next;
        switch(level)
        {
            case 'L': options.error_level = error_correction::level_L; break;
            case 'M': options.error_level = error_correction::level_M; break;
            case 'Q': options.error_level = error_correction::level_Q; break;
            case 'H': options.error_level = error_correction::level_H; break;
            default: return std::nullopt;
        }
        
        ++current;
        return std::pair{current, options};
    }

    [[nodiscard]] constexpr auto eci_number_option(arguments options, char** current, char** next)
        -> std::optional<std::pair<char**, arguments>>
    {
        if (next == nullptr)
            return std::nullopt;

        auto const eci_number = std::atoi(*next);
        if (eci_number < 0 || eci_number > 999999)
            return std::nullopt;

        options.eci_number = eci::assignment_number{eci_number};
        ++current;
        return std::pair{current, options};
    }

    [[nodiscard]] constexpr auto symbol_version_option(
        arguments options, char** current, char** next
    ) -> std::optional<std::pair<char**, arguments>>
    {
        if (next == nullptr)
            return std::nullopt;

        auto const version_number = std::atoi(*next);
        if (version_number < 1 || version_number > 40)
            return std::nullopt;

        options.version = symbol_version{version_number};
        ++current;
        return std::pair{current, options};
    }

    [[nodiscard]] auto read_message(std::istream& in)
    {
        using view = std::ranges::basic_istream_view<
            char, std::istream::char_type, std::istream::traits_type>;

        auto message = cx::vector<char>{};
        for (auto i : view{in})
            message.push_back(i);
        return message;
    }

    [[nodiscard]] constexpr auto parse_command_line(int argc, char** argv) 
        -> std::optional<arguments>
    {
        using callback_return_type = std::optional<std::pair<char**, arguments>>;
        using callback = callback_return_type(*)(arguments, char**, char**);
        using namespace std::literals;

        auto options = arguments{};

        auto const dispatch = std::array<std::pair<std::string_view, callback>, 3>{{
            {"--error_level"sv, error_level_option},
            {"--symbol_version"sv, symbol_version_option},
            {"--eci_number"sv, eci_number_option}
        }};

        for (auto i = argv, e = argv + argc; i != e; ++i)
        {
            auto const found = std::ranges::find_if(
                dispatch, 
                [current = std::string_view{*i}](auto i) { return i.first == current; });

            auto const result = found == end(dispatch) 
                ? message_option(options, i, i+1)
                : found->second(options, i, i+1);

            if (!result)
                return std::nullopt;

            i = result->first;
            options = result->second;
        }

        return options;
    }

    template<class Message>
    [[nodiscard]] constexpr auto do_create_symbol(
        Message&& message, std::optional<symbol_version> version, error_correction error_level)
    {
        return version 
            ? make_symbol(std::forward<Message>(message), *version, error_level)
            : make_symbol(std::forward<Message>(message), error_level);
    }

    template<class Message>
    [[nodiscard]] constexpr auto create_symbol(arguments const& args, Message const& message)
    {
        if (args.eci_number)
            return do_create_symbol(
                eci::view{*args.eci_number, message}, args.version, args.error_level);
        
        return do_create_symbol(message, args.version, args.error_level);
    }
}

int main(int argc, char** argv)
{
    auto const options = parse_command_line(argc-1, argv+1);
    if (!options)
    {
        print_usage(std::cerr, std::string_view{argv[0]});
        return 1;
    }
    print_options(std::cerr, *options);
    
    auto message = options->message;
    auto std_input = cx::vector<char>{};
    if (empty(options->message))
    {
        std_input = read_message(std::cin);
        message = std::string_view{cx::begin(std_input), cx::end(std_input)};
    }
    print_message(std::cerr, message);

    auto const symbol = create_symbol(*options, message);
    if (!symbol)
    {
        print_error(std::cerr, symbol.error());
        return 1;
    }

    auto s = symbol.value();
    std::cerr << "generated type   " << designator(s) << "\n";
    svg(std::cout, symbol.value());
}
