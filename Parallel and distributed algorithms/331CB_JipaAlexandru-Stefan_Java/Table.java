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
/*O tabela are nume, 2 vectori de stringuri cu numele coloanelor si tipurile
lor + plus o lista de linii din tabela (rezultata in urma insert-urilor)
*/
public class Table {
    String name;
    String[] columnNames;
    String[] columnTypes;
    ArrayList<Object> elements = new ArrayList<>();
    
    public Table(String name, String[] columnNames, String[] columnTypes) {
        this.name = name;
        this.columnNames = columnNames;
        this.columnTypes = columnTypes;
    }

    public Table() {
        
    }
    
    public String getName() {
        return name;
    }

    public String[] getColumnNames() {
        return columnNames;
    }

    public String[] getColumnTypes() {
        return columnTypes;
    }

    public void setName(String name) {
        this.name = name;
    }

    public void setColumnNames(String[] columnNames) {
        this.columnNames = columnNames;
    }

    public void setColumnTypes(String[] columnTypes) {
        this.columnTypes = columnTypes;
    }
    
}
