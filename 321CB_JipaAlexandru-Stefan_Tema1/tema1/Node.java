/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author Stefan Jipa
 */
public class Node {

    // folosesc 4 subnoduri pt ca for-ul poate avea 4 subprograme de executat
    public Node[] kids = new Node[4];
    public String value;
    int countParenthesis = 0;
    private int len;
    int i;

    // construiesc arborele recursiv
    public Node(String s) {
        value = s;
        countParenthesis = 0;
        len = value.length();

        // numar parantezele deschise si inchise ca sa ma aisgur ca am acelasi nr
        for (i = 0; i < len - 1; i++) {
            if (value.toCharArray()[i] == '[') {
                countParenthesis++;
            }

            if (value.toCharArray()[i] == ']') {
                countParenthesis--;
            }
            if (countParenthesis == 0) {
                break;
            }

        }
        // daca am o diferenta, elimin parantezele 
        if (countParenthesis > 0 && value.charAt(0) == '[') {
            value = new String(value.substring(1, len - 1));
            len = value.length();
        }

        // verific daca au mai ramas paranteze
        if (countParenthesis < 0) {
            value = new String(value.substring(0, len - 1));
            len = value.length();
        }

        // construiesc arborele, impartind fiecare element
        String[] elements = Main.Listsplitting(value);
        if (elements[0].equals("+")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = "+";
            return;
        }
        if (elements[0].equals("*")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = "*";
            return;
        }
        if (elements[0].equals("==")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = "==";
            return;
        }
        if (elements[0].equals("<")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = "<";
            return;
        }
        if (elements[0].equals("return")) {
            kids[0] = new Node(elements[1]);
            kids[1] = null;
            value = "return";
            return;
        }
        if (elements[0].equals("assert")) {
            kids[0] = new Node(elements[1]);
            kids[1] = null;
            value = "assert";
            return;
        }
        if (elements[0].equals("if")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            kids[2] = new Node(elements[3]);
            value = "if";
            return;
        }
        if (elements[0].equals("for")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            kids[2] = new Node(elements[3]);
            kids[3] = new Node(elements[4]);
            value = "for";
            return;
        }
        if (elements[0].equals(";")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = ";";
            return;
        }
        if (elements[0].equals("=")) {
            kids[0] = new Node(elements[1]);
            kids[1] = new Node(elements[2]);
            value = "=";
        }
    }
}
