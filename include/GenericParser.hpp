#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>

static std::string get_next_nonempty_line(std::ifstream& input_file) {
    std::string line;
    std::getline(input_file, line);
    while (line.empty()) { std::getline(input_file, line); }
    return line;
}

template <typename T>
struct DefaultStrToT {
    T operator()(std::istringstream input_file) {
        T res;
        input_file >> res;
        return res;
    }
};

struct BaseToken {
    bool has_value;
    virtual bool try_parse(std::ifstream& input_file);
};

template <typename T, class GetKeyAndValue>
class Token : public BaseToken {
public:
    Token(std::string_view key_, T& dest_) : BaseToken(key_), dest(dest_), has_value(false) { }
    inline T& get_dest() const { return dest; }

    inline bool try_parse(std::ifstream& input_file) override {
        auto pos = input_file.tellg();
        auto [k, t] = GetKeyAndValue()(input_file);
        if (k != key) {
            input_file.seekg(pos);
            return false;
        }
        has_value = true;
        dest = t;
        return true;
    }

private:
    std::string key;
    T& dest;
};

template <typename T, class GetKeyAndValue>
class TokenDependent : public Token<T, GetKeyAndValue> {
public:
    TokenDependent(std::string_view key_, T& dest_, BaseToken& master_) : Token<T, GetKeyAndValue>(key_, dest_), master(master_) { }

    inline bool try_parse(std::ifstream& input_file) override {
        if (!sizeTok.has_value) { return false; }
        return Token<T, GetKeyAndValue>(input_file);
    }

protected:
    BaseToken& master;
};

template <typename T, class GetKey, class GetT>
class TokenDependentSeq : public TokenDependent<T*, GetT> {
public:
    TokenDependentSeq(std::string_view key_, T*& dest_, BaseToken& sizeTok_, size_t* size_) : TokenDependent<T*, GetT>(key_, dest_, sizeTok_), size(size_) {
        dest = nullptr;
    }

    inline bool try_parse(std::ifstream& input_file) override {
        if (!master.has_value) { return false; }

        auto pos = input_file.tellg();
        if (GetKey()(input_file) != key) {
            input_file.seekg(pos);
            return false;
        }

        assert(dest == nullptr);
        dest = new T[size];
        has_value = true;

        for (size_t i = 0; i < size; ++i) { dest[i] = GetT()(input_file, dest[i]); }
        return true;
    }

private:
    size_t* size;
};

template <typename T>
struct TspLibKeyValue {
    auto operator()(std::istringstream input_file) {
        std::string key;
        std::string colon;
        T val;
        input_file >> key >> colon >> val;
        return std::make_pair(key, val);
    }
};

struct TspLibKey {
    auto operator()(std::istringstream input_file) {
        std::string key;
        std::string colon;
        input_file >> key >> colon;
        return key;
    }
};

template <typename T>
struct TspLibValue {
    auto operator()(std::istringstream input_file) {
        T val;
        input_file >> val;
        return val;
    }
};

class TspLibParser : public std::vector<BaseToken*> {

    void bind_NAME(std::string& dest) { add_token("NAME", dest); }
    void bind_COMMENT(std::string& dest) { add_token("COMMENT", dest); }
    void bind_TYPE(std::string& dest) { add_token("TYPE", dest); }
    void bind_DIMENSION(int& dest) { add_token("DIMENSION", dest); }
    void bind_EDGE_WEIGHT_TYPE(std::string& dest) { add_token("EDGE_WEIGHT_TYPE", dest); }
    void bind_CAPACITY(double& dest) { add_token("CAPACITY", dest); }
    void bind_DISTANCE(double& dest) { add_token("DISTANCE", dest); }
    void bind_SERVICE_TIME(double& dest) { add_token("SERVICE_TIME", dest); }

    void bind_NODE_COORD_SECTION(std::pair<double, double>*& dest) { add_token("NODE_COORD_SECTION", dest); }
    void bind_DEMAND_SECTION(double*& dest) { add_token("DEMAND_SECTION", dest); }
    void bind_DEPOT_SECTION(int& dest) { add_token("DEPOT_SECTION", dest); }

private:
    template <typename T>
    void add_token(std::string key, T& dest) {
        push_back(new Token<T, TspLibKeyValue<T>>(key, dest));
    }

    template <typename T>
    void add_seq_token(std::string key, T*& dest) {
        push_back(new TokenDependentSeq<T, TspLibKey, TspLibValue<T>>(key, dest));
    }
};


void generic_parser(std::string filepath, std::vector<BaseToken*>& tokens) {
    std::ifstream input_file(filepath);
    if (input_file.is_open()) {
        while (input_file.good()) {

            for (BaseToken* bt : tokens) {
                if (bt->try_parse(input_file)) { goto __break_and_continue__; }
            }

            throw std::string("Error while parsing line " + get_next_nonempty_line(input_file));

        __break_and_continue__:;
        }
    } else
        throw std::invalid_argument("Impossible to open instance file: " + filepath);
}