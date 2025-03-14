#pragma once

#include <vector>
#include <string>

#include <type_traits>

namespace  {

const std::string& quotes("\"");

}


namespace sql {

class column;

class column_value;
class SelectModel;

class Param
{
public:
    Param(const std::string& param) :
        _param(param) {}
    Param(const char* param) :
        _param(param) {}

public:
    std::string operator()() const { return param(); }
    inline std::string param() const { return _param; }

private:
    const std::string _param;
};

template<typename T>
inline std::string to_value(const T& data)
{
    return std::to_string(data);
}

template<size_t N>
inline std::string to_value(char const (&data)[N])
{
    std::string str("'");

    str.append(data);
    str.append("'");
    return str;
}

template<>
inline std::string to_value<std::string>(const std::string& data)
{
    std::string str("'");

    str.append(data);
    str.append("'");
    return str;
}

template<>
inline std::string to_value<const char*>(const char* const& data)
{
    std::string str("'");

    str.append(data);
    str.append("'");
    return str;
}

template<>
inline std::string to_value<Param>(const Param& data)
{
    return data();
}

template<>
inline std::string to_value<column>(const column& data);
template<>
inline std::string to_value<column_value>(const column_value& data);

/*
   template <>
   static std::string sql::to_value<time_t>(const time_t& data) {
    char buff[128] = {0};
    struct tm* ttime = localtime(&data);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", ttime);
    std::string str("'");
    str.append(buff);
    str.append("'");
    return str;
   }
 */

template<typename T>
void join_vector(std::string& result, const std::vector<T>& vec, const char* sep)
{
    size_t size = vec.size();

    for (size_t i = 0; i < size; ++i)
    {
        if (i < size - 1)
        {
            result.append(vec[i]);
            result.append(sep);
        }
        else
        {
            result.append(vec[i]);
        }
    }
}

class column
{
public:


    column() {}
    // alias
    column(const std::string& column_name, const std::string& alias = "", const std::string& as = "", const std::string& to_type = "")
    {
        if (!alias.empty())
            _cond.append(alias + ".");



        _cond.append(quotes + column_name + quotes);

        if (!to_type.empty())
            _cond.append("::" + to_type);

        if (!as.empty())
            _cond.append(" AS " + as);
    }

    column(const column& column_name, const std::string& alias = "", const std::string& as = "", const std::string& to_type = "")
    {
        if (!alias.empty())
            _cond.append(alias + ".");
        _cond.append(column_name.str());

        if (!to_type.empty())
            _cond.append("::" + to_type);

        if (!as.empty())
            _cond.append(" AS " + as);
    }

    column& operator()(const std::string& column_name, const std::string& alias = "", const std::string& as = "")
    {
        if (!alias.empty())
            _cond.append(alias + ".");
        _cond.append(quotes + column_name + quotes);

        if (!as.empty())
            _cond.append(" AS " + as);

        return *this;
    }

    virtual ~column() {}


    column& is_null()
    {
        _cond.append(" is null");
        return *this;
    }

    column& is_not_null()
    {
        _cond.append(" is not null");
        return *this;
    }

    template<typename T>
    column& in (const std::vector<T>& args) {
        size_t size = args.size();



        _cond.append(" in (");

        for (size_t i = 0; i < size; ++i)
        {
            if (i < size - 1)
            {
                _cond.append(to_value(args[i]));
                _cond.append(", ");
            }
            else
            {
                _cond.append(to_value(args[i]));
            }
        }
        _cond.append(")");

        return *this;
    }

    column& in (const std::string& in_column) {
        _cond.append(" in (");

        _cond.append(in_column);
        _cond.append(" ) ");
        return *this;
    }


    column& append(const std::string& data)
    {
        _cond.append(" || '" + data + "' ");
        return *this;
    }

    column& prepend(const std::string& data)
    {
        _cond.insert(0, " '" + data + "' || ");
        return *this;
    }

    // special characters such as   %,_ must contains in like_condition
    // template<>
    column& like(const std::string& like_condition)
    {
        if (like_condition.size() <= 0)
            return *this;

        _cond.append(" LIKE  '");
        _cond.append(like_condition);
        _cond.append("' ");


        return *this;
    }

    template<typename T>
    column& not_in(const std::vector<T>& args)
    {
        size_t size = args.size();

        if (size == 1)
        {
            _cond.append(" != ");
            _cond.append(to_value(args[0]));
        }
        else
        {
            _cond.append(" not in (");

            for (size_t i = 0; i < size; ++i)
            {
                if (i < size - 1)
                {
                    _cond.append(to_value(args[i]));
                    _cond.append(", ");
                }
                else
                {
                    _cond.append(to_value(args[i]));
                }
            }
            _cond.append(")");
        }
        return *this;
    }

    column& operator&&(column& condition)
    {
        std::string str("(");

        str.append(_cond);
        str.append(") and (");
        str.append(condition._cond);
        str.append(")");
        condition._cond = str;
        return condition;
    }

    column& operator||(column& condition)
    {
        std::string str("(");

        str.append(_cond);
        str.append(") or (");
        str.append(condition._cond);
        str.append(")");
        condition._cond = str;
        return condition;
    }

    column& operator&&(const std::string& condition)
    {
        _cond.append(" and ");
        _cond.append(condition);
        return *this;
    }

    column& operator||(const std::string& condition)
    {
        _cond.append(" or ");
        _cond.append(condition);
        return *this;
    }

    column& operator&&(const char* condition)
    {
        _cond.append(" and ");
        _cond.append(condition);
        return *this;
    }

    column& operator||(const char* condition)
    {
        _cond.append(" or ");
        _cond.append(condition);
        return *this;
    }

    template<typename T>
    column& operator==(const T& data)
    {
        _cond.append(" = ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator!=(const T& data)
    {
        _cond.append(" != ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator>=(const T& data)
    {
        _cond.append(" >= ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator<=(const T& data)
    {
        _cond.append(" <= ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator>(const T& data)
    {
        _cond.append(" > ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator<(const T& data)
    {
        _cond.append(" < ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator/(const T& data)
    {
        _cond.append(" / ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator+(const T& data)
    {
        _cond.append(" + ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator*(const T& data)
    {
        _cond.append(" * ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column& operator-(const T& data)
    {
        _cond.append(" - ");
        _cond.append(to_value(data));
        return *this;
    }

    const std::string& str() const
    {
        return _cond;
    }

    operator bool() {
        return true;
    }

private:
    std::string _cond;
};


class table
{
public:
    table(const std::string& table_name, const std::string& tablespace = "", const std::string& alias = "")
    {
        if (!tablespace.empty())
        {
            table_str_.append(tablespace);
            table_str_.append(".");
        }
        table_str_.append(quotes + table_name + quotes);

        if (!alias.empty())
            table_str_.append(" " + alias + " ");
        else
            table_str_.append(" ");
    }

    const std::string& str() const
    { return table_str_;}

private:
    std::string table_str_ = "";
};

class column_value
{
public:

    column_value(const std::string& column_value, const std::string& to_type = "", const std::string& as = "", const bool& is_value = false)
    {
        if (!is_value)
            _cond.append("'" + column_value + "'");
        else
            _cond.append(column_value);


        if (!to_type.empty())
            _cond.append("::" + to_type);

        if (!as.empty())
            _cond.append(" AS " +   as);
    }

    const std::string& str() const
    {
        return _cond;
    }

    template<typename T>
    column_value& operator>(const T& data)
    {
        _cond.append(" > ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column_value& operator<(const T& data)
    {
        _cond.append(" < ");
        _cond.append(to_value(data));
        return *this;
    }

    template<typename T>
    column_value& operator*(const T& data)
    {
        _cond.append(" * " + to_value(data));
        return *this;
    }

private:
    std::string _cond;
};

template<>
inline std::string to_value<column>(const column& data)
{
    return data.str();
}

inline std::string to_value(const sql::column_value& data)
{
    return data.str();
}

inline std::string to_value(const std::string& data)
{
    return data;
}

class SqlFunction
{
public:
    SqlFunction() :
        _sql_func("") {}
    virtual ~SqlFunction() {}

    virtual  std::string str() const = 0;

private:
    // SqlFunction(const SqlFunction& data)            = delete;
    SqlFunction& operator=(const SqlFunction& data) = delete;

protected:
    std::string _sql_func;
};


class existsStatement : public SqlFunction
{
public:

    existsStatement()
    {}

    template<typename T>
    existsStatement& exists(T subq, const std::string& as)
    {
        _sql_func.append("EXISTS (");
        _sql_func.append(to_value(subq));
        _sql_func.append(") ");

        if  (as.length() > 2)
            _sql_func.append(" AS " + as  + " ");

        return *this;
    }

    virtual   std::string  str() const override
    {
        return _sql_func;
    }
};

class caseStatement : public SqlFunction
{
public:
    caseStatement()
    {
        _sql_func.append(" END");
    }

    template<typename ... Args>
    caseStatement& case_sql(std::string as = "", std::string  case_else = "", Args&& ... conditionals)
    {
        if (case_else.length() > 2)
            _sql_func.insert(0, " ELSE '" + case_else + "'");

        if (!as.empty())
            _sql_func.append(" AS " + as);

        case_sql(conditionals ...);
        return *this;
    }

    template<typename T, typename N, typename ... Args>
    caseStatement& case_sql(std::pair<T, N> when, Args&& ... conditionals)
    {
        std::string pb(" WHEN ");

        pb.append(to_value(when.first));
        pb.append(" THEN  ");
        pb.append(to_value(when.second));

        _sql_func.insert(0, pb);
        case_sql(conditionals ...);
        return *this;
    }

    caseStatement& case_sql()
    {
        _sql_func.insert(0, "CASE ");
        return *this;
    }

    virtual   std::string  str() const override
    {
        return _sql_func;
    }

    virtual ~caseStatement() {}
};

class SqlWindowFunction : public SqlFunction
{
public:
    SqlWindowFunction()  {}
    virtual ~SqlWindowFunction() {}

    SqlWindowFunction& row_number(const sql::column& column
                                  , const sql::column&& partition =  sql::column()
                                  , const sql::column&& order     =  sql::column()
                                  , const bool& desc              = false
                                  , const std::string& as         = "")
    {
        return w_function("ROW_NUMBER", column.str(), partition.str(), order.str(), desc, as);
    }

    SqlWindowFunction& sum_over(const sql::column& column
                                , const std::string& partition = ""
                                , const std::string& order     = ""
                                , const bool& desc             = false
                                , const std::string& as        = "")
    {
        return w_function("SUM", column.str(), partition, order, desc,  as);
    }

    SqlWindowFunction& count_over(const sql::column& column
                                  , const std::string& partition = ""
                                  , const std::string& order     = ""
                                  , const bool& desc             = false
                                  , const std::string& as        = "")
    {
        return w_function("COUNT", column.str(), partition, order, desc, as);
    }

    SqlWindowFunction& count(const sql::column& column)
    {
        _sql_func.clear();
        _sql_func.append("COUNT(" + column.str() + ") ");

        return *this;
    }

    SqlWindowFunction& dense_rank(
        const std::string& partition = ""
        , const std::string& order   = ""
        , const bool& desc           = false
        , const std::string& as      = "")
    {
        return w_function("DENSE_RANK", "", partition, order, desc, as);
    }

    SqlWindowFunction& dense_rank(
        const sql::column& partition = sql::column()
        , const sql::column& order   = sql::column()
        , const bool& desc           = false
        , const std::string& as      = "")
    {
        return w_function("DENSE_RANK",  "", partition.str(), order.str(), desc, as);
    }

    SqlWindowFunction& dense_rank(
        const sql::column& partition                                    = sql::column()
        , const std::vector<std::pair<sql::column, bool>> order_by_desc = std::vector<std::pair<sql::column, bool>>
                                                                          ()
        , const std::string& as = "")
    {
        std::vector<std::pair<std::string, bool>> order_by_desc_string;

        for (int i = 0; i < order_by_desc.size(); i++)
        {
            order_by_desc_string.push_back(std::pair(order_by_desc.at(i).first.str(), order_by_desc.at(i).second));
        }

        // const std::vector<std::pair<std::string, bool>> const_order_by_desc_string = order_by_desc_string;
        return w_function("DENSE_RANK", "", partition.str(), order_by_desc_string, as);
    }

    SqlWindowFunction& dense_rank(
        const std::string& partition                                    = std::string()
        , const std::vector<std::pair<std::string, bool>> order_by_desc = std::vector<std::pair<std::string, bool>>
                                                                          ()
        , const std::string& as = "")
    {
        return w_function("DENSE_RANK", "", partition, order_by_desc, as);
    }

    SqlWindowFunction& rank(
        const std::string& partition = ""
        , const std::string& order   = ""
        , const bool& desc           = false
        , const std::string& as      = "")
    {
        return w_function("RANK", "", partition, order, desc, as);
    }

    SqlWindowFunction& rank(
        const sql::column& partition = sql::column()
        , const sql::column& order   = sql::column()
        , const bool& desc           = false
        , const std::string& as      = "")
    {
        return w_function("RANK",  "", partition.str(), order.str(), desc, as);
    }

    virtual   std::string  str() const override
    {
        return _sql_func;
    }

private:

    SqlWindowFunction& w_function(const std::string&  function_name,
                                  const std::string& column
                                  , const std::string& partition = ""
                                  , const std::string& order_by  = ""
                                  , const  bool desc             =  false
                                  , const std::string& as        = "")
    {
        _sql_func.clear();

        _sql_func.append(function_name);

        _sql_func.append("(");

        if (!column.empty() || (column.length() > 2))
            _sql_func.append(column);

        _sql_func.append(")");
        _sql_func.append(" OVER ");
        _sql_func.append("(");


        if (partition.length() > 2)
        {
            _sql_func.append("PARTITION BY ");
            _sql_func.append(partition);

            _sql_func.append(" ");
        }

        if  (order_by.length() > 2)
        {
            _sql_func.append("ORDER BY ");
            _sql_func.append(order_by);
            _sql_func.append(" ");
            desc ? _sql_func.append(" DESC ")
                 : _sql_func.append(" ASC ");
            _sql_func.append(" ) ");
        }

        if  (!as.empty())
            _sql_func.append(" AS " + as);

        return *this;
    }

    SqlWindowFunction& w_function(const std::string&  function_name,
                                  const std::string& column
                                  ,
                                  const std::string& partition = ""
                                  ,
                                  const std::vector<std::pair<std::string, bool>> order_by_desc = std::vector<std::pair<std::string, bool>>
                                                                                                  ()
                                  ,
                                  const std::string& as = "")
    {
        _sql_func.clear();
        _sql_func.append(function_name);

        _sql_func.append("(");

        if (!column.empty() || (column.length() > 2))
            _sql_func.append(column);

        _sql_func.append(")");
        _sql_func.append(" OVER ");
        _sql_func.append("(");


        if (partition.length() > 2)
        {
            _sql_func.append("PARTITION BY ");
            _sql_func.append(partition);

            _sql_func.append(" ");
        }

        if (order_by_desc.size() > 0)
        {
            _sql_func.append("ORDER BY ");

            for (int i  = 0; i  < order_by_desc.size(); i++)
            {
                if  (order_by_desc.at(i).first.length() > 2)
                {
                    _sql_func.append(order_by_desc.at(i).first);
                    _sql_func.append(" ");
                    order_by_desc.at(i).second ? _sql_func.append(" DESC , ")
                                               : _sql_func.append(" ASC , ");
                }
            }
            _sql_func = _sql_func.substr(0, _sql_func.size() - 3);

            _sql_func.append(" ) ");
        }

        if  (!as.empty())
            _sql_func.append(" AS " + as);

        return *this;
    }
};

class TimeFormatingFunction : public SqlFunction
{
public:
    TimeFormatingFunction() {}
    virtual ~TimeFormatingFunction() {}


    TimeFormatingFunction& to_timestamp(const column& data)
    {
        return t_function("to_timestamp", data);
    }

    virtual   std::string  str() const override
    {
        return _sql_func;
    }

private:
    TimeFormatingFunction& t_function(const std::string& name, const column& data)
    {
        _sql_func.append(name + "(" + data.str() + ")");
        return *this;
    }
};

class CastFunction : public SqlFunction
{
public:
    CastFunction(const column expression,

                 const  std::string& data_type = "",
                 const int& length = 30)
    {
        _sql_func = "CAST(";
        _sql_func.append(expression.str() +  " AS " + data_type + ") ");
    }

    std::string str() const override
    {
        return _sql_func;
    }
};




class RoundFunction : public SqlFunction
{
public:
    RoundFunction(const CastFunction expression,
                  const std::string& as = "",
                  const int& length = 30)
    {
        _sql_func = "ROUND(";
        _sql_func.append(expression.str() +  " , " + std::to_string(length) + ") ");

        if (!as.empty())
            _sql_func.append(" AS " + as);
    }

    RoundFunction(const column expression,
                  const std::string& as = "",
                  const int& length = 30)
    {
        _sql_func = "ROUND(";
        _sql_func.append(expression.str() +  " , " + std::to_string(length) + ") ");

        if (!as.empty())
            _sql_func.append(" AS " + as);
    }

    std::string str() const override
    {
        return _sql_func;
    }
};



class DataTypeFormatingFunction : public SqlFunction
{
public:
    DataTypeFormatingFunction(const std::string& as = "")
    {
        if (!as.empty())
            _as.append(" AS " + quotes + as + quotes);
    }

    std::string str() const override
    {
        return _sql_func + _as;
    }

    virtual ~DataTypeFormatingFunction() {}


    DataTypeFormatingFunction& to_char(const sql::TimeFormatingFunction& tf_func,
                                       const bool& is_text,
                                       const  std::string& format = "")
    {
        DataTypeFormatingFunction& text = dtf_function("to_char", tf_func.str(), is_text, format);


        return text;
    }

    DataTypeFormatingFunction& to_char(const column& data,
                                       const std::string& format = "")
    {
        return dtf_function("to_char", data.str(), true, format);
    }

    DataTypeFormatingFunction& to_char(const column_value& data,
                                       const std::string& format = "")
    {
        return dtf_function("to_char", data.str(), false, format);
    }

private:
    DataTypeFormatingFunction& dtf_function(const std::string& name,
                                            const std::string& data,
                                            const bool& is_column,
                                            const  std::string& format = "")
    {
        _sql_func.append(name);

        _sql_func.append("(");
        _sql_func.append(data);

        if (!format.empty())
            _sql_func.append(", '" + format + "'");
        _sql_func.append(")");
        return *this;
    }

    std::string _as;
};




class conditional_expressions : public SqlFunction
{
public:
    conditional_expressions(const std::string& as = "")
    {
        _sql_func.append(" COALESCE ( ");

        if (!as.empty())
            _as.append(" AS " +   as);
    }

    virtual  std::string str() const override
    {
        return _sql_func + _as;
    }

    virtual  ~conditional_expressions() {}

    template<typename T, typename ... Args>
    conditional_expressions& coalesce(const T& col, Args&& ... cols)
    {
        _sql_func.append(to_value(col) + " , ");
        coalesce(cols ...);
        return *this;
    }

private:
    conditional_expressions& coalesce()
    {
        // remove last comma
        if (_sql_func.size() > 4)
            _sql_func = _sql_func.substr(0, _sql_func.size() - 3);
        _sql_func.append(" ) ");
        return *this;
    }

    std::string _as = "";
};

class SqlModel
{
public:
    SqlModel() {}
    virtual ~SqlModel() {}

    virtual const std::string& str() = 0;
    const std::string& last_sql()
    {
        return _sql;
    }

private:
    //  SqlModel(const SqlModel& m)               = delete;
    SqlModel& operator=(const SqlModel& data) = delete;

protected:
    std::string _sql;
};

class SelectModel : public SqlModel
{
public:
    SelectModel() :
        _distinct(false) {}
    virtual ~SelectModel() {}

    template<typename ... Args>
    SelectModel& select(const std::string& str, Args&& ... columns)
    {
        const std::string& pb(quotes + str + quotes);

        _select_columns.push_back(pb);
        select(columns ...);
        return *this;
    }

    template<typename ... Args>
    SelectModel& select(const SqlFunction& sql_function, Args&& ... columns)
    {
        const std::string& pb = sql_function.str();

        _select_columns.push_back(pb);
        select(columns ...);
        return *this;
    }

    template<typename ... Args>
    SelectModel& select(std::pair<SelectModel, std::string> subquery, Args&& ... columns)
    {
        std::string pb(" ( ");

        pb.append(subquery.first.str());
        pb.append(" ) ");

        if (!subquery.second.empty())
        {
            pb.append(" AS ");
            pb.append(subquery.second);
        }

        _select_columns.push_back(pb);
        select(columns ...);
        return *this;
    }

    template<typename ... Args>
    SelectModel& select(const column column_struct, Args&& ... columns)
    {
        const std::string& pb = column_struct.str();

        _select_columns.push_back(pb);
        select(columns ...);
        return *this;
    }

    template<typename ... Args>
    SelectModel& select(const column_value data, Args&& ... columns)
    {
        const std::string& pb = data.str();

        _select_columns.push_back(pb);
        select(columns ...);
        return *this;
    }

    // for recursion
    SelectModel& select()
    {
        return *this;
    }

    SelectModel& distinct()
    {
        _distinct = true;
        return *this;
    }

    // template<typename ... Args>
    SelectModel& from(const std::string& table_name, const std::string& tablespace = "", const std::string& alias = "")
    {
        if (!tablespace.empty())
        {
            _table_name.append(tablespace);
            _table_name.append(".");
        }
        _table_name.append(quotes + table_name + quotes);

        if (!alias.empty())
            _table_name.append(" " + alias + " ");
        else
            _table_name.append(" ");


        return *this;
    }

    SelectModel& from_subquery(const std::string& table_name, const std::string& alias = "")
    {
        _table_name.append(table_name);

        if (!alias.empty())
            _table_name.append(" " + alias + " ");
        else
            _table_name.append(" ");


        return *this;
    }

    SelectModel& from(std::vector<SelectModel> selects,  const std::string& alias = "")
    {
        for (int i = 0; i < selects.size(); i++)
        {
            std::string subs_q = selects[i].str();

            _table_name.append(" ( " + subs_q + " )");

            if (i != selects.size())
                _table_name.append(" UNION ALL ");
        }

        if (!alias.empty())
            _table_name.append(" " + alias + " ");
        else
            _table_name.append(" ");

        return *this;
    }

    SelectModel& join_statement(const std::string& join_type,
                                const std::string& table_name,
                                const std::string& tablespace,
                                const std::string& alias,
                                const std::string& on_conditions)
    {
        //        std::vector<std::pair<std::string, std::vector<std::string>>> type;

        std::string join_type_and_table(" " + join_type + " ");

        if (!tablespace.empty())
            join_type_and_table.append(tablespace + ".");

        join_type_and_table.append(quotes + table_name + quotes);

        if (!alias.empty())
            join_type_and_table.append(" " + alias + " ");

        join_type_and_table.append("ON " + on_conditions);

        _join_type.push_back(join_type_and_table);

        return *this;
    }

    SelectModel& left_join(const std::string& table_name,
                           const column& on_conditions,
                           const std::string& tablespace = "",
                           const std::string& alias      = ""
                           )
    {
        join_statement("LEFT JOIN", table_name, tablespace, alias, on_conditions.str());

        return *this;
    }

    SelectModel& left_outer_join(const std::string& table_name,
                                 const column& on_conditions,
                                 const std::string& tablespace = "",
                                 const std::string& alias      = "")
    {
        join_statement("left outer join", table_name, tablespace, alias, on_conditions.str());

        return *this;
    }

    SelectModel& right_join(const std::string& table_name,
                            const column& on_conditions,
                            const std::string& tablespace = "",
                            const std::string& alias      = ""
                            )
    {
        join_statement(" right join ", table_name, tablespace, alias, on_conditions.str());
        return *this;
    }

    SelectModel& right_outer_join(const std::string& table_name,
                                  const column& on_conditions,
                                  const std::string& tablespace = "",
                                  const std::string& alias      = "")
    {
        join_statement(" RIGHT OUTER JOIN ", table_name, tablespace, alias, on_conditions.str());
        return *this;
    }

    SelectModel& full_join(const std::string& table_name,
                           const column& on_conditions,
                           const std::string& tablespace = "",
                           const std::string& alias      = "")
    {
        join_statement(" full join ", table_name, tablespace, alias, on_conditions.str());
        return *this;
    }

    SelectModel& full_outer_join(const std::string& table_name,
                                 const column& on_conditions,
                                 const std::string& tablespace = "",
                                 const std::string& alias      = "")
    {
        join_statement(" FULL OUTER JOIN ", table_name, tablespace, alias, on_conditions.str());
        return *this;
    }

    SelectModel& where(const std::string& condition)
    {
        _where_condition.push_back(condition);
        return *this;
    }

    SelectModel& where(const column& condition)
    {
        _where_condition.push_back(condition.str());
        return *this;
    }

    SelectModel& where(const  column_value& condition)
    {
        _where_condition.push_back(condition.str());
        return *this;
    }

    SelectModel& where_exists(std::vector<SelectModel> data)
    {
        std::string where_c;

        for (int i = 0; i < data.size(); i++)
        {
            where_c.append("(EXISTS (");
            where_c.append(data[i].str());
            where_c.append(" ) ) OR ");
        }
        where_c = where_c.substr(0, where_c.length() - 3);

        _where_condition.push_back(where_c);
        return *this;
    }

    SelectModel& where_not_exists(std::vector<SelectModel> data)
    {
        std::string where_c;

        for (int i = 0; i < data.size(); i++)
        {
            where_c.append("( NOT EXISTS (");
            where_c.append(data[i].str());
            where_c.append(" ) )  OR ");
        }
        where_c = where_c.substr(0, where_c.length() - 3);
        _where_condition.push_back(where_c);
        return *this;
    }

    template<typename T>
    SelectModel& where_between(const  column& cond, const T& begin_val, const  T& end_val)
    {
        std::string where_val;

        where_val.append(cond.str());
        std::string begin = std::to_string(begin_val);
        std::string end   = std::to_string(end_val);
        where_val.append(" BETWEEN " + begin + " AND " + end);
        _where_condition.push_back(where_val);
        return *this;
    }

    template<typename ... Args>
    SelectModel& group_by(const std::string& str, Args&& ... columns)
    {
        _groupby_columns.push_back(str);
        group_by(columns ...);
        return *this;
    }

    // for recursion
    SelectModel& group_by()
    {
        return *this;
    }

    SelectModel& having(const std::string& condition)
    {
        _having_condition.push_back(condition);
        return *this;
    }

    SelectModel& having(const column& condition)
    {
        _having_condition.push_back(condition.str());
        return *this;
    }

    SelectModel& order_by(const std::string& order_by,  const bool desc = false)
    {
        _order_by      = order_by;
        _order_by_desc = desc;
        return *this;
    }

    SelectModel& order_by(const column& order_by,  const bool desc = false)
    {
        _order_by      = to_value(order_by);
        _order_by_desc = desc;
        return *this;
    }

    template<typename T>
    SelectModel& limit(const T& limit)
    {
        _limit = std::to_string(limit);
        return *this;
    }

    template<typename T>
    SelectModel& limit(const T& offset, const T& limit)
    {
        _offset = std::to_string(offset);
        _limit  = std::to_string(limit);
        return *this;
    }

    template<typename T>
    SelectModel& offset(const T& offset)
    {
        _offset = std::to_string(offset);
        return *this;
    }

    virtual const std::string& str() override
    {
        _sql.clear();
        _sql.append(" SELECT ");

        if (_distinct)
            _sql.append(" DISTINCT ");
        join_vector(_sql, _select_columns, ", ");
        _sql.append(" FROM ");
        _sql.append(_table_name);

        if (!_join_type.empty())
        {
            for (std::string  join :_join_type)
            {
                _sql.append(" " + join + " ");
            }
        }

        if (!_where_condition.empty())
        {
            _sql.append(" WHERE ");
            join_vector(_sql, _where_condition, " AND ");
        }

        if (!_groupby_columns.empty())
        {
            _sql.append(" group by ");
            join_vector(_sql, _groupby_columns, ", ");
        }

        if (!_having_condition.empty())
        {
            _sql.append(" having ");
            join_vector(_sql, _having_condition, " and ");
        }

        if (!_order_by.empty())
        {
            _sql.append(" ORDER BY ");
            _sql.append(_order_by);

            if (_order_by_desc)
                _sql.append(" DESC ");
        }

        if (!_limit.empty())
        {
            _sql.append(" limit ");
            _sql.append(_limit);
        }

        if (!_offset.empty())
        {
            _sql.append(" offset ");
            _sql.append(_offset);
        }
        return _sql;
    }

    SelectModel& reset()
    {
        _select_columns.clear();
        _distinct = false;
        _groupby_columns.clear();
        _table_name.clear();
        _join_type.clear();
        // 64_join_on_condition.clear();
        _where_condition.clear();
        _having_condition.clear();
        _order_by.clear();
        _limit.clear();
        _offset.clear();
        return *this;
    }

    friend inline std::ostream& operator<<(std::ostream& out, SelectModel& mod)
    {
        out << mod.str();
        return out;
    }

protected:
    std::vector<std::string> _select_columns;
    bool _distinct;
    std::vector<std::string> _groupby_columns;
    std::string _table_name;
    std::vector<std::string> _join_type;

    // std::vector<std::string> _join_on_condition;
    std::vector<std::string> _where_condition;
    std::vector<std::string> _having_condition;
    std::string _order_by;
    bool _order_by_desc = false;
    std::string _limit;
    std::string _offset;
};

inline std::string to_value(SelectModel& data)
{
    return data.str();
}

class InsertModel : public SqlModel
{
public:
    InsertModel() {}
    virtual ~InsertModel() {}

    template<typename T>
    InsertModel& insert(const std::string& c, const T& data)
    {
        if constexpr
        (std::is_same<bool, T>::value)
        {
            _columns.push_back(quotes + c + quotes);
            std::string v_data = "FALSE";

            if (data) v_data =  "TRUE";
            _values.push_back(v_data);
        }
        else
            if constexpr (std::is_same<int, T>::value)
            {
                _columns.push_back(quotes + c + quotes);

                _values.push_back(std::to_string(data));
            }
            else
            {
                _columns.push_back(quotes + c + quotes);
                _values.push_back("'" + to_value(data) + "'");
            }
        return *this;
    }

    InsertModel& insert(const std::string& c)
    {
        _columns.push_back(quotes + c + quotes);
        _values.push_back(" ? ");
        return *this;
    }

    template<typename T>
    InsertModel& operator()(const std::string& c, const T& data)
    {
        return insert(c, data);
    }

    InsertModel& operator()(const std::string& c)
    {
        return insert(c);
    }

    InsertModel& into(const std::string& table_name, const std::string& tablespace = "")
    {
        _table_name.clear();

        if (!tablespace.empty())
            _table_name.append(tablespace + ".");
        _table_name.append(quotes + table_name + quotes);
        return *this;
    }

    InsertModel& replace(bool var)
    {
        _replace = var;
        return *this;
    }

    virtual const std::string& str() override
    {
        _sql.clear();
        std::string v_ss;

        if (_replace)
            _sql.append("insert or replace into ");
        else
            _sql.append("insert into ");

        _sql.append(_table_name);
        _sql.append("(");
        v_ss.append(" values(");
        size_t size = _columns.size();

        for (size_t i = 0; i < size; ++i)
        {
            if (i < size - 1)
            {
                _sql.append(_columns[i]);
                _sql.append(", ");
                v_ss.append(_values[i]);
                v_ss.append(", ");
            }
            else
            {
                _sql.append(_columns[i]);
                _sql.append(")");
                v_ss.append(_values[i]);
                v_ss.append(")");
            }
        }
        _sql.append(v_ss);
        return _sql;
    }

    InsertModel& reset()
    {
        _table_name.clear();
        _columns.clear();
        _values.clear();
        return *this;
    }

    friend inline std::ostream& operator<<(std::ostream& out, InsertModel& mod)
    {
        out << mod.str();
        return out;
    }

protected:
    bool _replace = false;
    std::string _table_name;
    std::vector<std::string> _columns;
    std::vector<std::string> _values;
};

template<>
inline InsertModel& InsertModel::insert(const std::string& c, const std::nullptr_t&)
{
    _columns.push_back(c);
    _values.push_back("null");
    return *this;
}

class UpdateModel : public SqlModel
{
public:
    UpdateModel() {}
    virtual ~UpdateModel() {}

    UpdateModel& update(const std::string& table_name)
    {
        _table_name = table_name;
        return *this;
    }

    template<typename T>
    UpdateModel& set(const std::string& c, const T& data)
    {
        std::string str(c);

        str.append(" = ");
        str.append(to_value(data));
        _set_columns.push_back(str);
        return *this;
    }

    template<typename T>
    UpdateModel& operator()(const std::string& c, const T& data)
    {
        return set(c, data);
    }

    UpdateModel& where(const std::string& condition)
    {
        _where_condition.push_back(condition);
        return *this;
    }

    UpdateModel& where(const column& condition)
    {
        _where_condition.push_back(condition.str());
        return *this;
    }

    virtual const std::string& str() override
    {
        _sql.clear();
        _sql.append("update ");
        _sql.append(_table_name);
        _sql.append(" set ");
        join_vector(_sql, _set_columns, ", ");
        size_t size = _where_condition.size();

        if (size > 0)
        {
            _sql.append(" WHERE ");
            join_vector(_sql, _where_condition, " and ");
        }
        return _sql;
    }

    UpdateModel& reset()
    {
        _table_name.clear();
        _set_columns.clear();
        _where_condition.clear();
        return *this;
    }

    friend inline std::ostream& operator<<(std::ostream& out, UpdateModel& mod)
    {
        out << mod.str();
        return out;
    }

protected:
    std::vector<std::string> _set_columns;
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

template<>
inline UpdateModel& UpdateModel::set(const std::string& c, const std::nullptr_t&)
{
    std::string str(c);

    str.append(" = null");
    _set_columns.push_back(str);
    return *this;
}

class DeleteModel : public SqlModel
{
public:
    DeleteModel() {}
    virtual ~DeleteModel() {}

    DeleteModel& _delete()
    {
        return *this;
    }

    template<typename ... Args>
    DeleteModel& from(const std::string& table_name, const std::string& tablespace = "")
    {
        if (!tablespace.empty())
        {
            _table_name.append(tablespace);
            _table_name.append(".");
        }
        _table_name.append(quotes + table_name + quotes);
        _table_name.append(" ");

        return *this;
    }

    DeleteModel& where(const std::string& condition)
    {
        _where_condition.push_back(condition);
        return *this;
    }

    DeleteModel& where(const column& condition)
    {
        _where_condition.push_back(condition.str());
        return *this;
    }

    virtual const std::string& str() override
    {
        _sql.clear();
        _sql.append("delete from ");
        _sql.append(_table_name);
        size_t size = _where_condition.size();

        if (size > 0)
        {
            _sql.append(" WHERE ");
            join_vector(_sql, _where_condition, " AND ");
        }
        return _sql;
    }

    DeleteModel& reset()
    {
        _table_name.clear();
        _where_condition.clear();
        return *this;
    }

    friend inline std::ostream& operator<<(std::ostream& out, DeleteModel& mod)
    {
        out << mod.str();
        return out;
    }

protected:
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

}
