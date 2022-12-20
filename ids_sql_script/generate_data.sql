/*****************************SALESORDERS DATA*********************************/
INSERT ALL
    INTO SalesOrders (client_surname,s_date) VALUES ('Winters',TO_DATE('24.12.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bender',TO_DATE('10.04.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bambini',TO_DATE('07.04.2016','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Rivas',TO_DATE('21.01.2015','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bauer',TO_DATE('20.01.2016','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Harris',TO_DATE('05.04.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bambini',TO_DATE('28.11.2016','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Rice',TO_DATE('26.01.2015','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bright',TO_DATE('23.01.2015','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Oracle',TO_DATE('12.11.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Ostap',TO_DATE('12.10.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Bambini',TO_DATE('02.01.2016','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Riverside corp.',TO_DATE('24.11.2015','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Vankovka',TO_DATE('14.02.2016','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('Vankovka',TO_DATE('15.08.2017','DD.MM.YYYY'))
    INTO SalesOrders (client_surname,s_date) VALUES ('East',TO_DATE('27.12.2016','DD.MM.YYYY'))
SELECT 1 FROM DUAL;

/*****************************BREEDS DATA**************************************/
INSERT ALL
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (66.43,28.21,'New Zealand','Alapaha Blue Blood Bulldog')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (58.34,25.50,'Australia','Andalusian Hound')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (72.09,26.43,'India','Anglo-Francais de Petite Venerie')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (60.73,25.34,'Germany','Appenzeller Sennenhund')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (72.80,26.00,'United Kingdom','Ariegeois')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (58.54,29.07,'Pakistan','Armant')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (72.50,28.36,'Spain','Bedlington Terrier')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (51.65,31.56,'Brazil','Billy')
    INTO Breeds (m_height,m_weight,origin,breed) VALUES (64.27,28.87,'Philippines','Black and Tan Coonhound')
SELECT 1 FROM DUAL;

/*****************************DOGS WITHOUT SALES DATA AND WITHOUT PARENTS****************************************/
INSERT ALL
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Leonard','M',TO_DATE('04.06.2009','DD.MM.YYYY'),87.92,'Alapaha Blue Blood Bulldog')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Kibo','M',TO_DATE('23.03.2010','DD.MM.YYYY'),93.24,'Andalusian Hound')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Paloma','F',TO_DATE('14.11.2009','DD.MM.YYYY'),52.51,'Alapaha Blue Blood Bulldog')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Tana','F',TO_DATE('21.07.2009','DD.MM.YYYY'),95.60,'Alapaha Blue Blood Bulldog')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Urielle','M',TO_DATE('10.12.2008','DD.MM.YYYY'),47.10,'Appenzeller Sennenhund')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Blossom','M',TO_DATE('30.08.2009','DD.MM.YYYY'),53.44,'Appenzeller Sennenhund')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Velma','F',TO_DATE('18.11.2009','DD.MM.YYYY'),75.31,'Bedlington Terrier')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Carl','F',TO_DATE('06.05.2008','DD.MM.YYYY'),76.84,'Billy')
    INTO Dogs (name,sex,birth_date,price,breed) VALUES ('Wade','M',TO_DATE('19.04.2008','DD.MM.YYYY'),38.09,'Armant')
SELECT 1 FROM DUAL;

/*****************************DOGS WITH SALES DATA AND WITHOUT PARENTS****************************************/
INSERT ALL
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Leon','M',TO_DATE('04.08.2009','DD.MM.YYYY'),69.92,3,69.92,'Alapaha Blue Blood Bulldog')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Kiban','M',TO_DATE('22.04.2010','DD.MM.YYYY'),87.24,4,80.00,'Andalusian Hound')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Pavlira','F',TO_DATE('18.09.2009','DD.MM.YYYY'),76.51,5,76.51,'Appenzeller Sennenhund')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Toni','F',TO_DATE('27.01.2009','DD.MM.YYYY'),58.60,6,58.60,'Bedlington Terrier')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Goriel','M',TO_DATE('12.09.2008','DD.MM.YYYY'),98.10,7,100.00,'Black and Tan Coonhound')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Bob','M',TO_DATE('20.08.2009','DD.MM.YYYY'),65.44,8,63.00,'Armant')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Dafnie','F',TO_DATE('11.12.2009','DD.MM.YYYY'),66.31,9,66.31,'Alapaha Blue Blood Bulldog')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Ciri','F',TO_DATE('02.12.2008','DD.MM.YYYY'),64.84,10,0.00,'Ariegeois')
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed) VALUES ('Bruce','M',TO_DATE('14.01.2008','DD.MM.YYYY'),52.09,11,52.09,'Ariegeois')
SELECT 1 FROM DUAL;

/*****************************DOGS WITHOUT SALES DATA AND WITH PARENTS****************************************/
INSERT ALL
    INTO Dogs (name,sex,birth_date,price,breed,mother_id) VALUES ('Brum','M',TO_DATE('04.06.2011','DD.MM.YYYY'),45.64,'Alapaha Blue Blood Bulldog',5)
    INTO Dogs (name,sex,birth_date,price,breed,mother_id) VALUES ('Michael','M',TO_DATE('23.03.2012','DD.MM.YYYY'),65.42,'Bedlington Terrier',9)
    INTO Dogs (name,sex,birth_date,price,breed,mother_id) VALUES ('Wiki','F',TO_DATE('14.11.2011','DD.MM.YYYY'),57.54,'Billy',10)
    INTO Dogs (name,sex,birth_date,price,breed,father_id) VALUES ('Dora','F',TO_DATE('21.07.2011','DD.MM.YYYY'),93.76,'Andalusian Hound',13)
    INTO Dogs (name,sex,birth_date,price,breed,father_id) VALUES ('Bob','M',TO_DATE('10.12.2011','DD.MM.YYYY'),102.10,'Alapaha Blue Blood Bulldog',3)
    INTO Dogs (name,sex,birth_date,price,breed,father_id) VALUES ('Max','M',TO_DATE('30.08.2012','DD.MM.YYYY'),53.44,'Alapaha Blue Blood Bulldog',3)
    INTO Dogs (name,sex,birth_date,price,breed,mother_id,father_id) VALUES ('Sharik','F',TO_DATE('18.11.2011','DD.MM.YYYY'),00.77,'Billy',9,13)
    INTO Dogs (name,sex,birth_date,price,breed,mother_id,father_id) VALUES ('Deoma','F',TO_DATE('06.05.2010','DD.MM.YYYY'),85.84,'Alapaha Blue Blood Bulldog',5,3)
    INTO Dogs (name,sex,birth_date,price,breed,mother_id,father_id) VALUES ('Meel','M',TO_DATE('19.04.2012','DD.MM.YYYY'),64.09,'Ariegeois',19,20)
SELECT 1 FROM DUAL;

/*****************************DOGS WITH SALES DATA AND WITH PARENTS****************************************/
INSERT ALL
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id) VALUES ('Jarda','M',TO_DATE('04.08.2013','DD.MM.YYYY'),12.43,12,25.92,'Billy',27)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id) VALUES ('Andrew','M',TO_DATE('22.04.2012','DD.MM.YYYY'),34.74,13,110.00,'Andalusian Hound',24)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id) VALUES ('Lila','F',TO_DATE('18.09.2014','DD.MM.YYYY'),24.00,14,20.00,'Appenzeller Sennenhund',14)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,father_id) VALUES ('Annie','F',TO_DATE('27.01.2013','DD.MM.YYYY'),45.00,15,123.00,'Bedlington Terrier', 22)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,father_id) VALUES ('Kickstarter','M',TO_DATE('12.09.2012','DD.MM.YYYY'),98.10,16,45.00,'Black and Tan Coonhound',7)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,father_id) VALUES ('Elon','M',TO_DATE('20.08.2014','DD.MM.YYYY'),99.00,17,00.00,'Armant',17)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id,father_id) VALUES ('Quebecca','F',TO_DATE('11.12.2016','DD.MM.YYYY'),24.44,17,24.31,'Alapaha Blue Blood Bulldog',18,11)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id,father_id) VALUES ('Oceania','F',TO_DATE('02.12.2015','DD.MM.YYYY'),73.84,16,37.00,'Ariegeois',19, 11)
    INTO Dogs (name,sex,birth_date,price,sales_id,sale_price,breed,mother_id,father_id) VALUES ('Mars','M',TO_DATE('14.01.2011','DD.MM.YYYY'),59.09,16,59.09,'Billy',10,11)
SELECT 1 FROM DUAL;

/*****************************EMPLOYEE DATA************************************/
INSERT ALL
    INTO Employee (name,surname,birth_date) VALUES ('Chancellor','Dodson',TO_DATE('18.06.1989','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Haviva','Daniel',TO_DATE('10.12.1983','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Dahlia','Melton',TO_DATE('17.10.1993','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Seth','Peters',TO_DATE('01.12.1982','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Lionel','Henson',TO_DATE('30.10.1989','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Walker','Cabrera',TO_DATE('11.06.1990','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Hector','Miranda',TO_DATE('10.05.1988','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Charles','Simpson',TO_DATE('29.05.1981','DD.MM.YYYY'))
    INTO Employee (name,surname,birth_date) VALUES ('Linus','Mckenzie',TO_DATE('14.05.1988','DD.MM.YYYY'))
SELECT 1 FROM Dual;

/*****************************VACCINATION DATA*********************************/
INSERT ALL
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('01.25.2022','MM.DD.YYYY'),'Trazodone HCl',198,3,9)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('02.26.2022','MM.DD.YYYY'),'Triamcinolone Acetonide',179,4,8)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('05.21.2020','MM.DD.YYYY'),'Enalapril Maleate',145,5,7)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('10.01.2020','MM.DD.YYYY'),'Amitriptyline HCl',137,6,7)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('03.27.2021','MM.DD.YYYY'),'Fluconazole',136,6,8)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('12.01.2020','MM.DD.YYYY'),'Risperidone',146,7,5)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('09.23.2020','MM.DD.YYYY'),'Suboxone',167,17,9)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('09.10.2021','MM.DD.YYYY'),'Carisoprodol',194,11,6)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('09.17.2020','MM.DD.YYYY'),'Clonazepam',193,15,4)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('12.06.2020','MM.DD.YYYY'),'Risperidone',146,17,10)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('01.13.2020','MM.DD.YYYY'),'Suboxone',167,17,11)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('04.23.2021','MM.DD.YYYY'),'Carisoprodol',194,11,6)
    INTO Vaccination (v_date,vac_type,days_valid,dog_id,employee_id) VALUES (TO_DATE('05.17.2020','MM.DD.YYYY'),'Clonazepam',193,8,11)
SELECT 1 FROM DUAL;

/*****************************Measures DATA*********************************/
INSERT ALL
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('01.25.2022','MM.DD.YYYY'),43,8,3,9)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('02.26.2022','MM.DD.YYYY'),90,10,4,8)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('05.21.2020','MM.DD.YYYY'),67,9,3,7)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('10.01.2020','MM.DD.YYYY'),54,7,6,9)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('03.27.2021','MM.DD.YYYY'),72,10,6,8)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('12.01.2020','MM.DD.YYYY'),84,8,7,5)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.23.2020','MM.DD.YYYY'),57,8,16,4)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.10.2021','MM.DD.YYYY'),58,8,10,6)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.17.2020','MM.DD.YYYY'),80,8,15,4)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('12.01.2020','MM.DD.YYYY'),84,8,17,6)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.23.2020','MM.DD.YYYY'),57,8,17,9)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.10.2021','MM.DD.YYYY'),58,8,11,6)
    INTO Measures (m_date,height,weight,dog_id,employee_id) VALUES (TO_DATE('09.17.2020','MM.DD.YYYY'),80,8,16,10)
SELECT 1 FROM DUAL;
