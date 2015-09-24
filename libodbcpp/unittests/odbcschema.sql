CREATE TABLE test(
		id int,
		fl numeric(5,2),
		string text,
		boolean bool,
		dt timestamp without time zone,
		ts interval);
INSERT INTO test VALUES(4, 123.45, 'some text', true, '2015-04-27 23:06:03', '1 day 14:13:12');
