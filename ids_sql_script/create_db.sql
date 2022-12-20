CREATE TABLE SalesOrders (
    sales_id    INTEGER NOT NULL,
    s_date      DATE NOT NULL,
    client_surname VARCHAR(100) NOT NULL,
    PRIMARY KEY (sales_id)
);

CREATE SEQUENCE s_ord_seq START WITH 3 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER s_ord_seq_tr
    BEFORE INSERT ON SalesOrders FOR EACH ROW
WHEN (NEW.sales_id IS NULL OR NEW.sales_id = 0)
BEGIN
    SELECT s_ord_seq.NEXTVAL INTO :NEW.sales_id FROM dual;
END;
/

CREATE TABLE Breeds (
    breed       VARCHAR(100) NOT NULL,
    m_height    NUMERIC(5,2) CHECK (m_height IS NOT NULL AND m_height > 0),
    m_weight    NUMERIC(5,2) CHECK (m_weight IS NOT NULL AND m_weight > 0),
    origin      VARCHAR(100),
    PRIMARY KEY (breed)
);

CREATE TABLE Dogs (
    dog_id      INTEGER NOT NULL,
    name        VARCHAR(100) NOT NULL,
    sex         CHAR(1) CHECK (sex IN ('F','M')),
    mother_id   INTEGER,
    father_id   INTEGER,
    birth_date  DATE,
    price       NUMERIC(30,2) CHECK (price IS NOT NULL AND price >= 0),
    sales_id    INTEGER,
    sale_price  NUMERIC(30,2) CHECK (sale_price >= 0),
    breed       VARCHAR(100) NOT NULL,

    PRIMARY KEY (dog_id),

    CONSTRAINT fk_mother_id
        FOREIGN KEY (mother_id)
        REFERENCES Dogs (dog_id)
        ON DELETE SET NULL,

    CONSTRAINT fk_father_id
        FOREIGN KEY (father_id)
        REFERENCES Dogs (dog_id)
        ON DELETE SET NULL,

    CONSTRAINT fk_sales_id
        FOREIGN KEY (sales_id)
        REFERENCES SalesOrders (sales_id)
        ON DELETE SET NULL,

    CONSTRAINT fk_breed
        FOREIGN KEY (breed)
        REFERENCES Breeds (breed)
);

CREATE SEQUENCE dogs_seq START WITH 3 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER dogs_seq_tr
    BEFORE INSERT ON Dogs FOR EACH ROW
WHEN (NEW.dog_id IS NULL OR NEW.dog_id = 0)
BEGIN
    SELECT dogs_seq.NEXTVAL INTO :NEW.dog_id FROM dual;
END;
/

CREATE OR REPLACE TRIGGER dog_insert_tr
    FOR INSERT OR UPDATE
    ON Dogs
COMPOUND TRIGGER
    TYPE mr_fr_bd_rt IS RECORD (
        mother_id   Dogs.mother_id%TYPE,
        father_id   Dogs.mother_id%TYPE,
        birth_date  Dogs.birth_date%TYPE
    );

    TYPE row_lvl_info_t IS TABLE OF mr_fr_bd_rt INDEX BY PLS_INTEGER;

     g_row_lvl_info   row_lvl_info_t;

    AFTER EACH ROW IS
    BEGIN
        g_row_lvl_info (g_row_lvl_info.COUNT + 1).mother_id   := :new.mother_id;
        g_row_lvl_info (g_row_lvl_info.COUNT).father_id       := :new.father_id;
        g_row_lvl_info (g_row_lvl_info.COUNT).birth_date      := :new.birth_date;
    END AFTER EACH ROW;

    AFTER STATEMENT IS
        res_count number := 0;
    BEGIN
        FOR indx IN 1 .. g_row_lvl_info.COUNT
         LOOP
            IF (g_row_lvl_info (indx).mother_id IS NOT NULL) THEN
                SELECT COUNT(*) INTO res_count
                FROM Dogs d
                WHERE d.dog_id = g_row_lvl_info (indx).mother_id AND d.sex LIKE 'M';
                IF (res_count > 0) THEN
                    raise_application_error (-20001,'Error: Dog mother is actually male');
                END IF;
                IF (g_row_lvl_info (indx).birth_date IS NOT NULL) THEN
                    SELECT COUNT(*) INTO res_count
                    FROM
                        (SELECT *
                        FROM Dogs d
                        WHERE d.dog_id = g_row_lvl_info (indx).mother_id AND d.birth_date IS NOT NULL) r
                    WHERE r.birth_date >= g_row_lvl_info (indx).birth_date;
                    IF (res_count > 0) THEN
                        raise_application_error (-20001,'Error: Mother is younger than her child');
                    END IF;
                END IF;
            END IF;


            IF (g_row_lvl_info(indx).father_id IS NOT NULL) THEN
                SELECT COUNT(*) INTO res_count
                FROM Dogs d
                WHERE d.dog_id = g_row_lvl_info (indx).father_id AND d.sex LIKE 'F';
                IF (res_count > 0) THEN
                    raise_application_error (-20001,'Error: Dog mother is actually male');
                END IF;
                IF (g_row_lvl_info (indx).birth_date IS NOT NULL) THEN
                    SELECT COUNT(*) INTO res_count
                    FROM
                        (SELECT *
                        FROM Dogs d
                        WHERE d.dog_id = g_row_lvl_info (indx).father_id AND d.birth_date IS NOT NULL) r
                    WHERE r.birth_date >= g_row_lvl_info (indx).birth_date;
                    IF (res_count > 0) THEN
                        raise_application_error (-20001,'Error: Father is younger than his child');
                    END IF;
                END IF;
            END IF;
         END LOOP;
     END AFTER STATEMENT;
END dog_insert_tr;
/

CREATE TABLE Employee (
    employee_id INTEGER NOT NULL,
    name        VARCHAR(100) CHECK(name IS NOT NULL AND REGEXP_LIKE(name,'^\w+$')),
    surname     VARCHAR(100) CHECK(surname IS NOT NULL AND REGEXP_LIKE(surname,'^\w+$')),
    birth_date  DATE NOT NULL,

    PRIMARY KEY (employee_id)
);

CREATE SEQUENCE empl_seq START WITH 3 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER empl_seq_tr
    BEFORE INSERT ON Employee FOR EACH ROW
WHEN (NEW.employee_id IS NULL OR NEW.employee_id = 0)
BEGIN
    SELECT empl_seq.NEXTVAL INTO :NEW.employee_id FROM dual;
END;
/

CREATE TABLE Vaccination (
    v_date      DATE NOT NULL,
    vac_type    VARCHAR(100) NOT NULL,
    days_valid  INTEGER CHECK (days_valid IS NOT NULL AND days_valid > 0),
    dog_id      INTEGER NOT NULL,
    employee_id INTEGER NOT NULL,

    PRIMARY KEY (v_date, vac_type, dog_id),

    CONSTRAINT fk_vac_dog_id
        FOREIGN KEY (dog_id)
        REFERENCES Dogs (dog_id),

    CONSTRAINT fk_vac_employee_id
        FOREIGN KEY (employee_id)
        REFERENCES Employee (employee_id)
);

CREATE TABLE Measures (
    m_date      DATE NOT NULL,
    height      NUMERIC(5,2) CHECK (height IS NOT NULL AND height > 0),
    weight      NUMERIC(5,2) CHECK (weight IS NOT NULL AND weight > 0),
    dog_id      INTEGER NOT NULL,
    employee_id INTEGER NOT NULL,

    PRIMARY KEY (m_date,dog_id),

    CONSTRAINT fk_mea_dog_id
        FOREIGN KEY (dog_id)
        REFERENCES Dogs (dog_id),

    CONSTRAINT fk_mea_employee_id
        FOREIGN KEY (employee_id)
        REFERENCES Employee (employee_id)
);
