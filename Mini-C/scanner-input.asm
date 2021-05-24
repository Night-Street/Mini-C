	include io32.inc
.data
	Student struct
	id dword ?
	age dword ?
	Student ends
	G1s Student 10 dup (<?>)
.code
	start:
	call mainv
	exit 0
createStudentii proc
	local t1:dword
	local t3:Student
	local L3ret7:Student
	local t2:dword
	mov eax, [ebp+12]
	mov t1, eax
	mov eax, t1
	mov L3ret7.id, eax
	mov eax, [ebp+16]
	mov t2, eax
	mov eax, t2
	mov L3ret7.age, eax
	mov eax, L3ret7.id
	mov t3.id, eax
	mov eax, L3ret7.age
	mov t3.age, eax
	mov edi, [ebp+8]
	mov eax, t3.id
	mov [edi+0], eax
	mov eax, t3.age
	mov [edi+4], eax
	ret 12
	ret 12
createStudentii endp
copyStudent7Student proc
	local t1:Student
	mov esi, [ebp+12]
	mov eax, [esi+0]
	mov t1.id, eax
	mov eax, [esi+4]
	mov t1.age, eax
	mov edi, [ebp+8]
	mov eax, t1.id
	mov [edi+0], eax
	mov eax, t1.age
	mov [edi+4], eax
	ret 8
	ret 8
copyStudent7Student endp
getSecondP7Student proc
	local t1:Student
	mov esi, [ebp+12]
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t1.id, edx
	mov edx, [eax+4]
	mov t1.age, edx
	mov edi, [ebp+8]
	mov eax, t1.id
	mov [edi+0], eax
	mov eax, t1.age
	mov [edi+4], eax
	ret 8
	ret 8
getSecondP7Student endp
print7Student proc
	local t1:dword
	local t2:dword
	mov esi, [ebp+8]
	mov eax, [esi+4]
	mov t1, eax
	push t1
	pop eax
	call dispsid
	mov eax, 32
	call dispc
	mov esi, [ebp+8]
	mov eax, [esi+8]
	mov t2, eax
	push t2
	pop eax
	call dispsid
	call dispcrlf
	ret 4
print7Student endp
mainv proc
	local t9:Student
	local t1:Student
	local t4:Student
	local t5:dword
	local t2:Student
	local t3:dword
	local t6:Student
	local t7:Student
	local t8:Student
	local t10:Student
	local t11:dword
	local t12:Student
	local t13:Student
	lea edi, G1s
	mov eax, 1
	imul eax, 8
	add eax, edi
	mov edx, 170801101
	mov [eax+0], edx
	lea edi, G1s
	mov eax, 1
	imul eax, 8
	add eax, edi
	mov edx, 22
	mov [eax+4], edx
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t1.id, edx
	mov edx, [eax+4]
	mov t1.age, edx
	lea eax, t1
	push eax
	call print7Student
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t4.id, edx
	mov edx, [eax+4]
	mov t4.age, edx
	mov eax, t4.age
	mov t5, eax
	push t5
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t2.id, edx
	mov edx, [eax+4]
	mov t2.age, edx
	mov eax, t2.id
	mov t3, eax
	push t3
	lea eax, t6
	push eax
	call createStudentii
	lea edi, G1s
	mov eax, 1
	imul eax, 8
	add eax, edi
	mov edx, t6.id
	mov [eax+0], edx
	mov edx, t6.age
	mov [eax+4], edx
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t7.id, edx
	mov edx, [eax+4]
	mov t7.age, edx
	lea eax, t7
	push eax
	call print7Student
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t8.id, edx
	mov edx, [eax+4]
	mov t8.age, edx
	lea eax, t8
	push eax
	lea eax, t9
	push eax
	call copyStudent7Student
	lea edi, G1s
	mov eax, 1
	imul eax, 8
	add eax, edi
	mov edx, t9.id
	mov [eax+0], edx
	mov edx, t9.age
	mov [eax+4], edx
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t10.id, edx
	mov edx, [eax+4]
	mov t10.age, edx
	lea eax, t10
	push eax
	call print7Student
	lea eax, G1s
	mov t11, eax
	push t11
	lea eax, t12
	push eax
	call getSecondP7Student
	lea edi, G1s
	mov eax, 1
	imul eax, 8
	add eax, edi
	mov edx, t12.id
	mov [eax+0], edx
	mov edx, t12.age
	mov [eax+4], edx
	lea esi, G1s
	mov eax, 1
	imul eax, 8
	add eax, esi
	mov edx, [eax+0]
	mov t13.id, edx
	mov edx, [eax+4]
	mov t13.age, edx
	lea eax, t13
	push eax
	call print7Student
	ret 0
mainv endp
	end start
