#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include "odbc-bind.h"
#include "odbc-param.h"
#include "odbc-param_fwd.h"
#include <algorithm>
#include <column.h>
#include <glibmm/ustring.h>
#include <sql.h>
#include <sqlext.h>
#include <vector>

namespace ODBC {
	class SelectCommand;
	class Column : public virtual Bind, public virtual DB::Column {
	public:
		Column(SelectCommand *, const Glib::ustring & s, unsigned int i);
		virtual ~Column() override = default;
		void bind();
		virtual void * rwDataAddress() = 0;
		virtual bool resize();

		[[nodiscard]] bool isNull() const override;
		virtual void apply(DB::HandleField &) const override = 0;

		const SelectCommand * selectCmd;

	protected:
		virtual const Param * meAsAParam() const = 0;
	};
	class CharArrayColumn : public Column, public Param {
	public:
		using CharArray = std::vector<char>;
		CharArrayColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i, SQLULEN sizeHint) :
			DB::Column(n, i), Column(sc, n, i)
		{
			data.resize(std::max<SQLULEN>(sizeHint, 64) + 1);
		}
		SQLSMALLINT
		ctype() const override
		{
			return SQL_C_CHAR;
		}
		SQLSMALLINT
		stype() const override
		{
			return SQL_CHAR;
		}
		SQLULEN
		size() const override
		{
			return data.size();
		}
		SQLSMALLINT
		dp() const override
		{
			return 0;
		}
		const void *
		dataAddress() const override
		{
			return &data.front();
		}
		void *
		rwDataAddress() override
		{
			return &data.front();
		}
		void operator=(const Glib::ustring & d);
		bool resize() override;
		void apply(DB::HandleField &) const override;

	protected:
		virtual const Param *
		meAsAParam() const override
		{
			return this;
		}
		CharArray data;
	};
	class SignedIntegerColumn : public Column, public SignedIntegerParam {
	public:
		SignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
			DB::Column(n, i), Column(sc, n, i)
		{
		}
		SQLSMALLINT
		ctype() const override
		{
			return SignedIntegerParam::ctype();
		}
		SQLULEN
		size() const override
		{
			return SignedIntegerParam::size();
		}
		void *
		rwDataAddress() override
		{
			return &data;
		}
		const Param *
		meAsAParam() const override
		{
			return this;
		}
		void apply(DB::HandleField &) const override;
	};
#ifdef COMPLETENESS
	class UnsignedIntegerColumn : public Column, public UnsignedIntegerParam {
	public:
		UnsignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) : Column(sc, n, i) { }
		const Param *
		meAsAParam() const override
		{
			return this;
		}
	};
#endif
	class FloatingPointColumn : public Column, public FloatingPointParam {
	public:
		FloatingPointColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
			DB::Column(n, i), Column(sc, n, i)
		{
		}
		SQLSMALLINT
		ctype() const override
		{
			return FloatingPointParam::ctype();
		}
		SQLULEN
		size() const override
		{
			return FloatingPointParam::size();
		}
		void *
		rwDataAddress() override
		{
			return &data;
		}
		const Param *
		meAsAParam() const override
		{
			return this;
		}
		void apply(DB::HandleField &) const override;
	};
	class IntervalColumn : public Column, public IntervalParam {
	public:
		IntervalColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) : DB::Column(n, i), Column(sc, n, i)
		{
		}
		SQLSMALLINT
		ctype() const override
		{
			return IntervalParam::ctype();
		}
		SQLULEN
		size() const override
		{
			return IntervalParam::size();
		}
		void *
		rwDataAddress() override
		{
			return &data;
		}
		const Param *
		meAsAParam() const override
		{
			return this;
		}
		void apply(DB::HandleField &) const override;
	};
	class TimeStampColumn : public Column, public TimeStampParam {
	public:
		TimeStampColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
			DB::Column(n, i), Column(sc, n, i)
		{
		}
		SQLSMALLINT
		ctype() const override
		{
			return TimeStampParam::ctype();
		}
		SQLULEN
		size() const override
		{
			return TimeStampParam::size();
		}
		void *
		rwDataAddress() override
		{
			return &data;
		}
		const Param *
		meAsAParam() const override
		{
			return this;
		}
		void apply(DB::HandleField &) const override;
	};
}

#endif
