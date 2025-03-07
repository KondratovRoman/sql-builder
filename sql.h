#pragma once

#include <vector>
#include <string>

namespace  {

const std::string& quotes("\"");

}


namespace sql {

class column;

class column_value;

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

    // alias
    column(const std::string& column_name, const std::string& alias = "", const std::string& as = "")
    {
        if (!alias.empty())
            _cond.append(alias + ".");
        _cond.append(quotes + column_name + quotes);

        if (!as.empty())
            _cond.append(" as " + as);
    }

    column& operator()(const std::string& column_name, const std::string& alias = "", const std::string& as = "")
    {
        if (!alias.empty())
            _cond.append(alias + ".");
        _cond.append(quotes + column_name + quotes);

        if (!as.empty())
            _cond.append(" as " + as);

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


        //        if (size == 1)
        //        {
        //            _cond.append(" = ");
        //            _cond.append(to_value(args[0]));
        //        }
        //        else
        //        {
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
            //            }
            _cond.append(")");
        }
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
    {}

private:
    std::string table_str_ = "";
};

class column_value
{
public:

    column_value(const std::string& column_value, const std::string& to_type = "", const std::string& as = "", const bool& is_null = false)
    {
        if (!is_null)
            _cond.append("'" + column_value + "'");
        else
            _cond.append(column_value);


        if (!to_type.empty())
            _cond.append(" ::" + to_type);

        if (!as.empty())
            _cond.append(" as " + as);
    }

    const std::string& str() const
    {
        return _cond;
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

class SqlFunction
{
public:
    SqlFunction() :
        _sql_func("") {}
    virtual ~SqlFunction() {}

    virtual  std::string str() const = 0;

private:
    SqlFunction(const SqlFunction& data)            = delete;
    SqlFunction& operator=(const SqlFunction& data) = delete;

protected:
    std::string _sql_func;
};

class SqlWindowFunction : public SqlFunction
{
public:
    SqlWindowFunction()  {}
    virtual ~SqlWindowFunction() {}

    SqlWindowFunction& row_number(const std::string& column
                                  , const std::string& partition = ""
                                  , const std::string& order     = ""
                                  , const bool& desc             = false)
    {
        return w_function("ROW_NUMBER", column, partition, order, desc);
    }

    virtual   std::string  str() const override
    {
        return _sql_func;
    }

private:

    SqlWindowFunction& w_function(const std::string&  function_name, const std::string& column
                                  , const std::string& partition = ""
                                  , const std::string& order     = ""
                                  , const bool& desc             = false)
    {
        _sql_func.clear();


        _sql_func.append(function_name);


        _sql_func.append("(");

        if (!column.empty())
            _sql_func.append(quotes + column + quotes);

        _sql_func.append(")");
        _sql_func.append(" OVER ");
        _sql_func.append("(");


        if (!partition.empty())
        {
            _sql_func.append("PARTITION BY ");
            _sql_func.append(quotes + partition + quotes);

            _sql_func.append(" ");
        }

        if (!order.empty())
        {
            _sql_func.append("ORDER BY ");
            _sql_func.append(quotes + order + quotes);
            _sql_func.append(" ");
        }

        if (!partition.empty() || !order.empty())
            desc ? _sql_func.append(" DESC ")
                 : _sql_func.append(" ASC ");
        _sql_func.append(")");
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

class DataTypeFormatingFunction : public SqlFunction
{
public:
    DataTypeFormatingFunction(const std::string& as = "")
    {
        if (!as.empty())
            _as.append(" as " + quotes + as + quotes);
    }

    std::string str() const override
    {
        // std::string result = _sql_func;

        // result = result.append(_as);
        return _sql_func + _as;    // CRASHES WTF
    }

    virtual ~DataTypeFormatingFunction() {}


    DataTypeFormatingFunction& to_char(const sql::TimeFormatingFunction& tf_func,
                                       const bool& is_text,
                                       const  std::string& format = "")
    {
        DataTypeFormatingFunction& text = dtf_function("to_char", tf_func.str(), is_text, format);
        std::string text_SDF            = text.str();

        return text;
    }

    DataTypeFormatingFunction& to_char(const column& data,
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
            _as.append(" as " + quotes + as + quotes);
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
    SqlModel(const SqlModel& m)               = delete;
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

        _join_type.push_back(std::make_pair(join_type_and_table, on_conditions));

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
        join_statement(" right outer join ", table_name, tablespace, alias, on_conditions.str());
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

    SelectModel& order_by(const std::string& order_by)
    {
        _order_by = order_by;
        return *this;
    }

    SelectModel& order_by(const column& order_by)
    {
        _order_by = to_value(order_by);
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
        _sql.append("select ");

        if (_distinct)
            _sql.append(" DISTINCT ");
        join_vector(_sql, _select_columns, ", ");
        _sql.append(" FROM ");
        _sql.append(_table_name);

        //        if (!_join_type.empty())
        //        {
        //            _sql.append(" ");
        //            _sql.append(_join_type);
        //            _sql.append(" ");
        //            _sql.append(_join_table);
        //        }

        //        if (!_join_on_condition.empty())
        //        {
        //            _sql.append(" on ");
        //            join_vector(_sql, _join_on_condition, " and ");
        //        }

        if (!_join_type.empty())
        {
            for (std::pair<std::string, std::string> join :_join_type)
            {
                _sql.append(" " + join.first + " ");
                std::string on = join.second;
                _sql.append(" ON ");

                _sql.append(join.second);
            }
        }

        if (!_where_condition.empty())
        {
            _sql.append(" where ");
            join_vector(_sql, _where_condition, " and ");
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
            _sql.append(" order by ");
            _sql.append(_order_by);
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
    std::vector<std::pair<std::string, std::string>> _join_type;

    // std::vector<std::string> _join_on_condition;
    std::vector<std::string> _where_condition;
    std::vector<std::string> _having_condition;
    std::string _order_by;
    std::string _limit;
    std::string _offset;
};



class InsertModel : public SqlModel
{
public:
    InsertModel() {}
    virtual ~InsertModel() {}

    template<typename T>
    InsertModel& insert(const std::string& c, const T& data)
    {
        _columns.push_back(quotes + c + quotes);
        _values.push_back(to_value(data));
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
            _sql.append(" where ");
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
        //        assert(table_name.empty());
        //        assert(!_table_name.empty());


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
            _sql.append(" where ");
            join_vector(_sql, _where_condition, " and ");
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
