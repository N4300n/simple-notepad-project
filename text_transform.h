#ifndef TEXT_TRANSFORM_H
#define TEXT_TRANSFORM_H

#include <cctype>
#include <string>
#include <utility>

class text_transform {
public:
    virtual ~text_transform() = default;

    [[nodiscard]] std::string name() const { return transform_name; }
    [[nodiscard]] virtual std::string apply(const std::string& text) const = 0;

protected:
    explicit text_transform(std::string name)
        : transform_name(std::move(name))
    {
    }

private:
    std::string transform_name;
};

class uppercase_transform final : public text_transform {
public:
    uppercase_transform()
        : text_transform("To Uppercase")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        for (char& ch : result) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        }
        return result;
    }
};

class lowercase_transform final : public text_transform {
public:
    lowercase_transform()
        : text_transform("To Lowercase")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        for (char& ch : result) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return result;
    }
};

class capitalize_transform final : public text_transform {
public:
    capitalize_transform()
        : text_transform("Capitalize")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        bool capitalize_next = true;
        for (char& ch : result) {
            unsigned char uch = static_cast<unsigned char>(ch);
            if (std::isspace(uch)) {
                capitalize_next = true;
            } else if (capitalize_next && std::isalpha(uch)) {
                ch = static_cast<char>(std::toupper(uch));
                capitalize_next = false;
            } else {
                ch = static_cast<char>(std::tolower(uch));
            }
        }
        return result;
    }
};

class sentence_case_transform final : public text_transform {
public:
    sentence_case_transform()
        : text_transform("Sentence Case")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        bool capitalize_next = true;
        for (char& ch : result) {
            unsigned char uch = static_cast<unsigned char>(ch);
            if (capitalize_next && std::isalpha(uch)) {
                ch = static_cast<char>(std::toupper(uch));
                capitalize_next = false;
            } else {
                ch = static_cast<char>(std::tolower(uch));
            }
            if (ch == '.') {
                capitalize_next = true;
            }
        }
        return result;
    }
};

class swap_case_transform final : public text_transform {
public:
    swap_case_transform()
        : text_transform("Swap Case")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        for (char& ch : result) {
            unsigned char uch = static_cast<unsigned char>(ch);
            if (std::islower(uch)) {
                ch = static_cast<char>(std::toupper(uch));
            } else if (std::isupper(uch)) {
                ch = static_cast<char>(std::tolower(uch));
            }
        }
        return result;
    }
};

#endif