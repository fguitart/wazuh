$existingAcl = Get-Acl -Path 'C:\Program Files (x86)\ossec-agent'
$permissions = $env:username, 'Read,Modify', 'ContainerInherit,ObjectInherit', 'None', 'Allow'
$rule = New-Object -TypeName System.Security.AccessControl.FileSystemAccessRule -ArgumentList $permissions
$existingAcl.SetAccessRule($rule)
$existingAcl | Set-Acl -Path 'C:\Program Files (x86)\ossec-agent'