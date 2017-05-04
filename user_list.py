#!/usr/bin/env python
# -*- coding: utf-8 -*-

def auth_login(usuario, password):

#Lista de usuario y contrasenas, ver si es conveniente pasar
#esto a una DB cifrada.
    users =[ ['administrador','admin' ],\
             ['ayudante'     ,'otro'  ], \
             ['biocl'        ,'biocl2']
           ]


    for i in range(0,len(users)):
        set(users[i])

    for i in range(0,len(users)):
        if usuario in users[i][0]:
            if password in users[i][1]:
                return True
            else:
                return False
