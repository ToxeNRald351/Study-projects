/*select vaccination date of all dogs that were sold in 09.04.14 - 19.01.15 timespan*/
/*3 tables connected*/
SELECT d.dog_id Dog_ID, d.name, TO_CHAR(v.v_date, 'DD.MM.YYYY') Vac_Date, v.vac_type Vac_Type
FROM Dogs d, Vaccination v, SalesOrders s
WHERE   d.dog_id = v.dog_id AND
        d.sales_id = s.sales_id AND 
        s.s_date BETWEEN TO_DATE('01.01.2015', 'DD.MM.YYYY') AND TO_DATE('31.12.2016', 'DD.MM.YYYY');

/*select all emloyees' id's, who vaccinated male dogs*/
/*2 tables connection, IN, clause GROUP BY with aggregation function COUNT()*/
SELECT e.employee_id, e.name, COUNT(v.dog_id) Number_of_Male_Dogs
FROM Vaccination v, Employee e
WHERE v.employee_id = e.employee_id AND v.dog_id IN (
    SELECT d.dog_id
    FROM Dogs d
    WHERE d.sex = 'M')
GROUP BY e.employee_id, e.name;

/* Select all client surnames, who owns female dog*/
/*2 tables connection, clause GROUP BY with aggregation function COUNT()*/
SELECT s.client_surname Client_surname, COUNT(sales_id)
FROM Salesorders s
WHERE EXISTS (
              SELECT d.sales_id
              FROM Dogs d
              WHERE d.sex = 'F'AND d.sales_id = s.sales_id
          )
GROUP BY Client_surname;

/**/
SELECT e.employee_id, e.name, COUNT(m.dog_id) Count_Of_Measurements
FROM Measures m, Employee e
WHERE m.employee_id = e.employee_id
GROUP BY e.employee_id, e.name
HAVING COUNT(m.dog_id) = (
    SELECT MAX(c_of_m.c_of_m_d)
    FROM (
         SELECT COUNT(m.dog_id) c_of_m_d
         FROM Measures m, Employee e
         WHERE m.employee_id = e.employee_id
         GROUP BY e.employee_id
         ) c_of_m
);
