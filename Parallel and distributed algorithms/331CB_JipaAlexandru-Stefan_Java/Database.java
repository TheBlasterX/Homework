/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

import java.util.ArrayList;

/**
 *
 * @author Stefan
 */
public class Database implements MyDatabase {

    // vector de tabele
    ArrayList<Table> tables = new ArrayList<>();
    // vector de nume de tabele
    ArrayList<String> tablenames = new ArrayList<>();

    public Database() {
    }

    @Override
    public void initDb(int numWorkerThreads) {
    }

    @Override
    public void stopDb() {
    }

    // functie de afisare a tuturor tabelelor pentru testare
    // in fisierul MyDatabase.java am pus antetul acestei functii
    public void printTables() {
        for (int i = 0; i < tables.size(); i++) {
            System.out.println(tables.get(i).name);
            for (int j = 0; j < tables.get(i).columnNames.length; j++) {
                System.out.print(tables.get(i).columnNames[j] + "("
                        + tables.get(i).columnTypes[j] + ") ");
            }
            System.out.println("");
            for (int j = 0; j < tables.get(i).elements.size(); j++) {
                System.out.print(tables.get(i).elements.get(j) + " ");
            }
            System.out.println("");
        }
    }

    @Override
    public void createTable(String tableName, String[] columnNames,
            String[] columnTypes) {
        // creeaza tabelul dat cu coloanele si tipurile lor
        tables.add(new Table(tableName, columnNames, columnTypes));
//creez un vector cu numele tabelelor pentru a accesa rapid tabela cu numele "X"
        tablenames.add(tableName);
    }

    @Override
    public ArrayList<ArrayList<Object>> select(String tableName,
            String[] operations, String condition) {
        // vector in care pun numele coloanelor in ordinea operatiilor de executat
        ArrayList<String> cols = new ArrayList<>();
        for (int i = 0; i < operations.length; i++) {
            if (operations[i].contains("(")) {
                if (operations[i].contains("count")) {
                    cols.add(operations[i].substring(6,
                            operations[i].length() - 1));
                } else {
                    cols.add(operations[i].substring(4,
                            operations[i].length() - 1));
                }
            } else {
                cols.add(operations[i]);
            }
        }
        // lista de liste intoarsa de funtie
        ArrayList<ArrayList<Object>> arrays = new ArrayList<>();
        Object[] parse_cond = condition.split(" "); // conditia
        String column = parse_cond[0].toString();  // numele coloanie
        String comparator = parse_cond[1].toString(); // comparator
        String type = parse_cond[2].toString(); // valoarea
        String val = null;
        // obtin coloana din tabel peste care se aplica conditia
        int pos = 0;
        for (int i = 0; i <
            tables.get(tablenames.indexOf(tableName)).columnNames.length; i++) {
            if (tables.get(tablenames.indexOf(tableName)).columnNames[i].equals(column)) {
                pos = i;
                break;
            }
        }
        // lista ce va contine numarul liniilor pe care conditia este true
        ArrayList<Integer> sel_rows = new ArrayList<>();
        // parcurg coloanele din tabela cu numele cautat
        for (int i = 0; i < tables.get(tablenames.indexOf(tableName)).elements.size(); i++) {
            Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(i).toString().split(" ");
        // parsez elementul din coloana pe care se va aplica conditia
            for (int j = pos; j < from_table.length; j += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                if (pos == 0) {
                    val = from_table[j].toString().substring(1, from_table[j].toString().length() - 1);
                } else {
                    val = from_table[j].toString().substring(0, from_table[j].toString().length() - 1);
                }   // aplic conditia
                if (comparator.equals("==")) {
                    if (val.equals(type)) {
                        sel_rows.add(i);
                    }
                } else if (comparator.equals("<")) {
                    if (Integer.parseInt(val) < Integer.parseInt(type)) {
                        sel_rows.add(i);
                    }
                } else if (comparator.equals(">")) {
                    if (Integer.parseInt(val) > Integer.parseInt(type)) {
                        sel_rows.add(i);
                    }
                } else if (comparator.equals("")) { // daca conditia e vida
                    sel_rows.add(i);
                }
            }
        }
        // aplic operatiile din select
        for (int i = 0; i < operations.length; i++) {
        // lista rezultata dupa fiecare operatie
            ArrayList<Object> values = new ArrayList<>();
            // operatie de tip selectare toata coloana
            if (!operations[i].contains("(")) {
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                    // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(operations[i])) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            } // adaug elementul din coloana in lista
                            values.add(val);
                        }
                    }
                }
            // operatia de tip count
            } else if (operations[i].contains("count")) {
                int ok = 0;
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                         // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(cols.get(i))) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            }
                            ok++;  // numar coloanele permise
                        }
                    }
                }
                values.add(ok);  // adaug numarul de coloane in lista
            } else if (operations[i].contains("sum")) {
                int sum = 0;
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                    // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(cols.get(i))) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            }
                            sum += Integer.parseInt(val); // fac suma
                        }
                    }
                }
                values.add(sum); // adaug suma in lista
            } else if (operations[i].contains("avg")) {
                int sum = 0;
                int ok = 0;
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                    // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(cols.get(i))) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            }
                            sum += Integer.parseInt(val); // fac suma
                            ok++; // numar elementele
                        }
                    }
                }
                values.add(sum / ok); // adaug media ca intreg in lista
            } else if (operations[i].contains("min")) {
                int min = Integer.MAX_VALUE;
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                         // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(cols.get(i))) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            }
                            if (Integer.parseInt(val) < min) {
                                min = Integer.parseInt(val); // fac minim
                            }
                        }
                    }
                }
                values.add(min); // adaug minimul in lista
            } else if (operations[i].contains("max")) {
                int max = Integer.MIN_VALUE;
                for (int j = 0; j < tables.get(tablenames.indexOf(tableName)).elements.size(); j++) {
                    Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(j).toString().split(" ");
                         // coloane pe care se pot aplica operatiile
                    if (sel_rows.contains(j)) {
                        for (int k = 0; k < tables.get(tablenames.indexOf(tableName)).columnNames.length; k++) {
                            if (tables.get(tablenames.indexOf(tableName)).columnNames[k].equals(cols.get(i))) {
                                pos = k;
                                break;
                            }
                        }
                        for (int k = pos; k < from_table.length; k += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                            if (pos == 0) {
                                val = from_table[k].toString().substring(1, from_table[k].toString().length() - 1);
                            } else {
                                val = from_table[k].toString().substring(0, from_table[k].toString().length() - 1);
                            }
                            if (Integer.parseInt(val) > max) {
                                max = Integer.parseInt(val); // fac maxim
                            }
                        }
                    }
                }
                values.add(max); // adaug maximul in lista
            }
            // adaug lista operatiei respective in lista de liste
            arrays.add(values);
        }
        // daca lista e goala intorc santinela
        return arrays; // intorc lista de liste
    }

    @Override
    public void update(String tableName, ArrayList<Object> values, String condition) {
        Object[] parse_cond = condition.split(" "); // parsez conditia
        String column = parse_cond[0].toString(); // numele coloanei
        String comparator = parse_cond[1].toString(); // comparator
        String type = parse_cond[2].toString(); // valoarea
        String val = null;
        // obtin coloana din tabel peste care se aplica conditia
        int pos = 0;
        for (int i = 0; i < tables.get(tablenames.indexOf(tableName)).columnNames.length; i++) {
            if (tables.get(tablenames.indexOf(tableName)).columnNames[i].equals(column)) {
                pos = i;
                break;
            }
        }
        // caut in tabela cu numele dat
        for (int i = 0; i < tables.get(tablenames.indexOf(tableName)).elements.size(); i++) {
            Object[] from_table = tables.get(tablenames.indexOf(tableName)).elements.get(i).toString().split(" ");
            // parsez elementul din coloana pe care se va aplica conditia
            for (int j = pos; j < from_table.length; j += tables.get(tablenames.indexOf(tableName)).columnNames.length) {
                if (pos == 0) {
                    val = from_table[j].toString().substring(1, from_table[j].toString().length() - 1);
                } else {
                    val = from_table[j].toString().substring(0, from_table[j].toString().length() - 1);
                } // aplic conditia
                if (comparator.equals("==")) { // daca e ==
                    if (val.equals(type)) {
                        tables.get(tablenames.indexOf(tableName)).elements.set(i, values);
                    }
                } else if (comparator.equals("<")) { // daca e <
                    if (Integer.parseInt(val) < Integer.parseInt(type)) {
                        tables.get(tablenames.indexOf(tableName)).elements.set(i, values);
                    }
                } else if (comparator.equals(">")) { // daca e >
                    if (Integer.parseInt(val) > Integer.parseInt(type)) {
                        tables.get(tablenames.indexOf(tableName)).elements.set(i, values);
                    }
                }
            }
        }
    }

    @Override
    public void insert(String tableName, ArrayList<Object> values) {
        // insereaza in tabel o noua intrare la sfarsitul tabelului
        tables.get(tablenames.indexOf(tableName)).elements.add(values);
    }

    @Override
    public void startTransaction(String tableName) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void endTransaction(String tableName) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

}
