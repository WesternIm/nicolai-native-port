
/mnt/data/m27_work/msi_extract/flat/mtsyc32.dll:     file format pei-i386


Disassembly of section .text:

102159c0 <.text+0x2149c0>:
102159c0:	e9 e6 02 00 00       	jmp    0x10215cab
102159c5:	68 78 cf 75 10       	push   0x1075cf78
102159ca:	6a 02                	push   0x2
102159cc:	50                   	push   eax
102159cd:	8d 84 24 24 01 00 00 	lea    eax,[esp+0x124]
102159d4:	50                   	push   eax
102159d5:	e8 c6 93 f8 ff       	call   0x1019eda0
102159da:	83 c4 10             	add    esp,0x10
102159dd:	85 c0                	test   eax,eax
102159df:	0f 85 02 01 00 00    	jne    0x10215ae7
102159e5:	8b 8c 24 48 02 00 00 	mov    ecx,DWORD PTR [esp+0x248]
102159ec:	53                   	push   ebx
102159ed:	db 44 24 1c          	fild   DWORD PTR [esp+0x1c]
102159f1:	8b 91 04 3d 00 00    	mov    edx,DWORD PTR [ecx+0x3d04]
102159f7:	55                   	push   ebp
102159f8:	d8 5a 60             	fcomp  DWORD PTR [edx+0x60]
102159fb:	df e0                	fnstsw ax
102159fd:	f6 c4 41             	test   ah,0x41
10215a00:	75 74                	jne    0x10215a76
10215a02:	8a 4c 24 1c          	mov    cl,BYTE PTR [esp+0x1c]
10215a06:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215a09:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a0c:	02 ca                	add    cl,dl
10215a0e:	8a 56 09             	mov    dl,BYTE PTR [esi+0x9]
10215a11:	50                   	push   eax
10215a12:	51                   	push   ecx
10215a13:	52                   	push   edx
10215a14:	e8 17 03 00 00       	call   0x10215d30
10215a19:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215a1d:	8a 56 10             	mov    dl,BYTE PTR [esi+0x10]
10215a20:	8b f8                	mov    edi,eax
10215a22:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a25:	53                   	push   ebx
10215a26:	55                   	push   ebp
10215a27:	50                   	push   eax
10215a28:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a2b:	02 c8                	add    cl,al
10215a2d:	51                   	push   ecx
10215a2e:	52                   	push   edx
10215a2f:	e8 fc 02 00 00       	call   0x10215d30
10215a34:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215a38:	8a 56 17             	mov    dl,BYTE PTR [esi+0x17]
10215a3b:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215a3f:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a42:	53                   	push   ebx
10215a43:	55                   	push   ebp
10215a44:	50                   	push   eax
10215a45:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a48:	02 c8                	add    cl,al
10215a4a:	51                   	push   ecx
10215a4b:	52                   	push   edx
10215a4c:	e8 df 02 00 00       	call   0x10215d30
10215a51:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215a55:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215a59:	83 c4 3c             	add    esp,0x3c
10215a5c:	85 c0                	test   eax,eax
10215a5e:	0f 84 26 fd ff ff    	je     0x1021578a
10215a64:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215a67:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215a6b:	53                   	push   ebx
10215a6c:	55                   	push   ebp
10215a6d:	50                   	push   eax
10215a6e:	8a 46 01             	mov    al,BYTE PTR [esi+0x1]
10215a71:	e9 32 02 00 00       	jmp    0x10215ca8
10215a76:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215a79:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
10215a7d:	8a 4e 09             	mov    cl,BYTE PTR [esi+0x9]
10215a80:	52                   	push   edx
10215a81:	02 06                	add    al,BYTE PTR [esi]
10215a83:	50                   	push   eax
10215a84:	51                   	push   ecx
10215a85:	e8 a6 02 00 00       	call   0x10215d30
10215a8a:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215a8d:	8a 4e 10             	mov    cl,BYTE PTR [esi+0x10]
10215a90:	53                   	push   ebx
10215a91:	55                   	push   ebp
10215a92:	8b f8                	mov    edi,eax
10215a94:	8a 44 24 30          	mov    al,BYTE PTR [esp+0x30]
10215a98:	52                   	push   edx
10215a99:	8a 16                	mov    dl,BYTE PTR [esi]
10215a9b:	02 c2                	add    al,dl
10215a9d:	50                   	push   eax
10215a9e:	51                   	push   ecx
10215a9f:	e8 8c 02 00 00       	call   0x10215d30
10215aa4:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215aa7:	8a 4e 17             	mov    cl,BYTE PTR [esi+0x17]
10215aaa:	53                   	push   ebx
10215aab:	55                   	push   ebp
10215aac:	89 44 24 4c          	mov    DWORD PTR [esp+0x4c],eax
10215ab0:	8a 44 24 44          	mov    al,BYTE PTR [esp+0x44]
10215ab4:	52                   	push   edx
10215ab5:	8a 16                	mov    dl,BYTE PTR [esi]
10215ab7:	02 c2                	add    al,dl
10215ab9:	50                   	push   eax
10215aba:	51                   	push   ecx
10215abb:	e8 70 02 00 00       	call   0x10215d30
10215ac0:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215ac4:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215ac8:	83 c4 3c             	add    esp,0x3c
10215acb:	85 c0                	test   eax,eax
10215acd:	0f 84 b7 fc ff ff    	je     0x1021578a
10215ad3:	8a 44 24 14          	mov    al,BYTE PTR [esp+0x14]
10215ad7:	8a 0e                	mov    cl,BYTE PTR [esi]
10215ad9:	8a 56 02             	mov    dl,BYTE PTR [esi+0x2]
10215adc:	53                   	push   ebx
10215add:	55                   	push   ebp
10215ade:	02 c1                	add    al,cl
10215ae0:	52                   	push   edx
10215ae1:	50                   	push   eax
10215ae2:	e9 c4 01 00 00       	jmp    0x10215cab
10215ae7:	8b 4c 24 2c          	mov    ecx,DWORD PTR [esp+0x2c]
10215aeb:	8b 94 24 48 02 00 00 	mov    edx,DWORD PTR [esp+0x248]
10215af2:	db 44 24 18          	fild   DWORD PTR [esp+0x18]
10215af6:	8a 84 0c 18 01 00 00 	mov    al,BYTE PTR [esp+ecx*1+0x118]
10215afd:	3c 3c                	cmp    al,0x3c
10215aff:	8b 82 04 3d 00 00    	mov    eax,DWORD PTR [edx+0x3d04]
10215b05:	d8 58 60             	fcomp  DWORD PTR [eax+0x60]
10215b08:	df e0                	fnstsw ax
10215b0a:	0f 85 b2 01 00 00    	jne    0x10215cc2
10215b10:	f6 c4 41             	test   ah,0x41
10215b13:	8b 44 24 3c          	mov    eax,DWORD PTR [esp+0x3c]
10215b17:	0f 85 cc 00 00 00    	jne    0x10215be9
10215b1d:	85 c0                	test   eax,eax
10215b1f:	75 51                	jne    0x10215b72
10215b21:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b24:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215b28:	8a 46 04             	mov    al,BYTE PTR [esi+0x4]
10215b2b:	53                   	push   ebx
10215b2c:	55                   	push   ebp
10215b2d:	51                   	push   ecx
10215b2e:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b31:	89 7c 24 48          	mov    DWORD PTR [esp+0x48],edi
10215b35:	02 d1                	add    dl,cl
10215b37:	52                   	push   edx
10215b38:	50                   	push   eax
10215b39:	e8 f2 01 00 00       	call   0x10215d30
10215b3e:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b41:	8a 54 24 28          	mov    dl,BYTE PTR [esp+0x28]
10215b45:	53                   	push   ebx
10215b46:	55                   	push   ebp
10215b47:	51                   	push   ecx
10215b48:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b4b:	8b f8                	mov    edi,eax
10215b4d:	8a 46 0b             	mov    al,BYTE PTR [esi+0xb]
10215b50:	02 d1                	add    dl,cl
10215b52:	52                   	push   edx
10215b53:	50                   	push   eax
10215b54:	e8 d7 01 00 00       	call   0x10215d30
10215b59:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b5c:	8a 54 24 3c          	mov    dl,BYTE PTR [esp+0x3c]
10215b60:	53                   	push   ebx
10215b61:	55                   	push   ebp
10215b62:	51                   	push   ecx
10215b63:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b66:	02 d1                	add    dl,cl
10215b68:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
10215b6c:	8a 46 12             	mov    al,BYTE PTR [esi+0x12]
10215b6f:	52                   	push   edx
10215b70:	eb 49                	jmp    0x10215bbb
10215b72:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b75:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215b79:	8a 46 06             	mov    al,BYTE PTR [esi+0x6]
10215b7c:	53                   	push   ebx
10215b7d:	55                   	push   ebp
10215b7e:	51                   	push   ecx
10215b7f:	02 56 01             	add    dl,BYTE PTR [esi+0x1]
10215b82:	52                   	push   edx
10215b83:	50                   	push   eax
10215b84:	e8 a7 01 00 00       	call   0x10215d30
10215b89:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215b8c:	8a 54 24 28          	mov    dl,BYTE PTR [esp+0x28]
10215b90:	53                   	push   ebx
10215b91:	55                   	push   ebp
10215b92:	51                   	push   ecx
10215b93:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215b96:	8b f8                	mov    edi,eax
10215b98:	8a 46 0d             	mov    al,BYTE PTR [esi+0xd]
10215b9b:	02 d1                	add    dl,cl
10215b9d:	52                   	push   edx
10215b9e:	50                   	push   eax
10215b9f:	e8 8c 01 00 00       	call   0x10215d30
10215ba4:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215ba7:	8a 54 24 3c          	mov    dl,BYTE PTR [esp+0x3c]
10215bab:	53                   	push   ebx
10215bac:	55                   	push   ebp
10215bad:	51                   	push   ecx
10215bae:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215bb1:	02 d1                	add    dl,cl
10215bb3:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
10215bb7:	8a 46 14             	mov    al,BYTE PTR [esi+0x14]
10215bba:	52                   	push   edx
10215bbb:	50                   	push   eax
10215bbc:	e8 6f 01 00 00       	call   0x10215d30
10215bc1:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215bc5:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215bc9:	83 c4 3c             	add    esp,0x3c
10215bcc:	85 c0                	test   eax,eax
10215bce:	0f 84 b6 fb ff ff    	je     0x1021578a
10215bd4:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215bd7:	8a 54 24 14          	mov    dl,BYTE PTR [esp+0x14]
10215bdb:	53                   	push   ebx
10215bdc:	55                   	push   ebp
10215bdd:	51                   	push   ecx
10215bde:	8a 4e 01             	mov    cl,BYTE PTR [esi+0x1]
10215be1:	02 d1                	add    dl,cl
10215be3:	52                   	push   edx
10215be4:	e9 c2 00 00 00       	jmp    0x10215cab
10215be9:	85 c0                	test   eax,eax
10215beb:	75 4e                	jne    0x10215c3b
10215bed:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215bf1:	8a 16                	mov    dl,BYTE PTR [esi]
10215bf3:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215bf6:	53                   	push   ebx
10215bf7:	55                   	push   ebp
10215bf8:	02 ca                	add    cl,dl
10215bfa:	8a 56 04             	mov    dl,BYTE PTR [esi+0x4]
10215bfd:	50                   	push   eax
10215bfe:	51                   	push   ecx
10215bff:	52                   	push   edx
10215c00:	89 7c 24 50          	mov    DWORD PTR [esp+0x50],edi
10215c04:	e8 27 01 00 00       	call   0x10215d30
10215c09:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215c0d:	8a 56 0b             	mov    dl,BYTE PTR [esi+0xb]
10215c10:	8b f8                	mov    edi,eax
10215c12:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c15:	53                   	push   ebx
10215c16:	55                   	push   ebp
10215c17:	50                   	push   eax
10215c18:	8a 06                	mov    al,BYTE PTR [esi]
10215c1a:	02 c8                	add    cl,al
10215c1c:	51                   	push   ecx
10215c1d:	52                   	push   edx
10215c1e:	e8 0d 01 00 00       	call   0x10215d30
10215c23:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215c27:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215c2b:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c2e:	8a 56 12             	mov    dl,BYTE PTR [esi+0x12]
10215c31:	53                   	push   ebx
10215c32:	55                   	push   ebp
10215c33:	50                   	push   eax
10215c34:	8a 06                	mov    al,BYTE PTR [esi]
10215c36:	02 c8                	add    cl,al
10215c38:	51                   	push   ecx
10215c39:	eb 48                	jmp    0x10215c83
10215c3b:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215c3f:	8a 16                	mov    dl,BYTE PTR [esi]
10215c41:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c44:	53                   	push   ebx
10215c45:	55                   	push   ebp
10215c46:	02 ca                	add    cl,dl
10215c48:	8a 56 06             	mov    dl,BYTE PTR [esi+0x6]
10215c4b:	50                   	push   eax
10215c4c:	51                   	push   ecx
10215c4d:	52                   	push   edx
10215c4e:	e8 dd 00 00 00       	call   0x10215d30
10215c53:	8a 4c 24 28          	mov    cl,BYTE PTR [esp+0x28]
10215c57:	8a 56 0d             	mov    dl,BYTE PTR [esi+0xd]
10215c5a:	8b f8                	mov    edi,eax
10215c5c:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c5f:	53                   	push   ebx
10215c60:	55                   	push   ebp
10215c61:	50                   	push   eax
10215c62:	8a 06                	mov    al,BYTE PTR [esi]
10215c64:	02 c8                	add    cl,al
10215c66:	51                   	push   ecx
10215c67:	52                   	push   edx
10215c68:	e8 c3 00 00 00       	call   0x10215d30
10215c6d:	8a 4c 24 3c          	mov    cl,BYTE PTR [esp+0x3c]
10215c71:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
10215c75:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c78:	8a 56 14             	mov    dl,BYTE PTR [esi+0x14]
10215c7b:	53                   	push   ebx
10215c7c:	55                   	push   ebp
10215c7d:	50                   	push   eax
10215c7e:	8a 06                	mov    al,BYTE PTR [esi]
10215c80:	02 c8                	add    cl,al
10215c82:	51                   	push   ecx
10215c83:	52                   	push   edx
10215c84:	e8 a7 00 00 00       	call   0x10215d30
10215c89:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
10215c8d:	8b 44 24 74          	mov    eax,DWORD PTR [esp+0x74]
10215c91:	83 c4 3c             	add    esp,0x3c
10215c94:	85 c0                	test   eax,eax
10215c96:	0f 84 ee fa ff ff    	je     0x1021578a
10215c9c:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215c9f:	8a 4c 24 14          	mov    cl,BYTE PTR [esp+0x14]
10215ca3:	53                   	push   ebx
10215ca4:	55                   	push   ebp
10215ca5:	50                   	push   eax
10215ca6:	8a 06                	mov    al,BYTE PTR [esi]
10215ca8:	02 c8                	add    cl,al
10215caa:	51                   	push   ecx
10215cab:	e8 e0 00 00 00       	call   0x10215d90
10215cb0:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
10215cb4:	83 c4 10             	add    esp,0x10
10215cb7:	2b c8                	sub    ecx,eax
10215cb9:	89 4c 24 34          	mov    DWORD PTR [esp+0x34],ecx
10215cbd:	e9 c8 fa ff ff       	jmp    0x1021578a
10215cc2:	f6 c4 41             	test   ah,0x41
10215cc5:	53                   	push   ebx
10215cc6:	55                   	push   ebp
10215cc7:	75 10                	jne    0x10215cd9
10215cc9:	8a 56 01             	mov    dl,BYTE PTR [esi+0x1]
10215ccc:	8a 44 24 1c          	mov    al,BYTE PTR [esp+0x1c]
10215cd0:	8a 4e 02             	mov    cl,BYTE PTR [esi+0x2]
10215cd3:	02 d0                	add    dl,al
10215cd5:	51                   	push   ecx
10215cd6:	52                   	push   edx
10215cd7:	eb 0d                	jmp    0x10215ce6
10215cd9:	8a 46 02             	mov    al,BYTE PTR [esi+0x2]
10215cdc:	8a 4c 24 1c          	mov    cl,BYTE PTR [esp+0x1c]
10215ce0:	50                   	push   eax
10215ce1:	8a 06                	mov    al,BYTE PTR [esi]
10215ce3:	02 c8                	add    cl,al
10215ce5:	51                   	push   ecx
10215ce6:	e8 a5 00 00 00       	call   0x10215d90
10215ceb:	8b 54 24 44          	mov    edx,DWORD PTR [esp+0x44]
10215cef:	83 c4 10             	add    esp,0x10
10215cf2:	03 c2                	add    eax,edx
10215cf4:	50                   	push   eax
10215cf5:	8d 44 24 54          	lea    eax,[esp+0x54]
10215cf9:	68 5c cf 75 10       	push   0x1075cf5c
10215cfe:	50                   	push   eax
10215cff:	ff 15 54 a1 23 10    	call   DWORD PTR ds:0x1023a154
10215d05:	8b 4c 24 38          	mov    ecx,DWORD PTR [esp+0x38]
10215d09:	8d 94 24 24 01 00 00 	lea    edx,[esp+0x124]
10215d10:	51                   	push   ecx
10215d11:	8d 44 24 60          	lea    eax,[esp+0x60]
10215d15:	52                   	push   edx
10215d16:	50                   	push   eax
10215d17:	e8 84 8f f8 ff       	call   0x1019eca0
10215d1c:	83 c4 18             	add    esp,0x18
10215d1f:	e9 22 fb ff ff       	jmp    0x10215846
