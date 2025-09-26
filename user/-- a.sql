-- a. Cartesian product with WHERE clause
SELECT e.first_name, e.last_name, e.deptno, d.dname
FROM employees e, departments d
WHERE e.deptno = d.deptno;

-- b. JOIN with USING clause
SELECT e.first_name, e.last_name, e.deptno, d.dname
FROM employees e
JOIN departments d
USING (deptno);

-- c. JOIN with ON clause
SELECT e.first_name, e.last_name, e.deptno, d.dname
FROM employees e
JOIN departments d
ON e.deptno = d.deptno;

-- d. NATURAL JOIN clause
SELECT e.first_name, e.last_name, e.deptno, d.dname
FROM employees e
NATURAL JOIN departments d;

-- e. Is there a difference in the output of the 4 methods above?
-- If the join condition is correct and there are no duplicate column names (except deptno), the output will be the same for all four methods.

-- f. What if the department ID was stored with different names in both tables, which of the methods will be more appropriate?
-- The JOIN with ON clause is most appropriate, as you can explicitly specify the column names:
-- Example:
-- ON e.department_id = d.dept_id