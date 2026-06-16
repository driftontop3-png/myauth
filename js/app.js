const state = {
  currentUser: null,
  licenses: [],
  stats: { total: 0, unused: 0, used: 0, banned: 0 },
  selectedLang: "csharp",
  searchQuery: "",
  apiBaseUrl: `${window.location.origin}/api/1.3/`,
};

const elements = {
  authView: document.getElementById("auth-view"),
  appView: document.getElementById("app-view"),
  loginForm: document.getElementById("login-form"),
  registerForm: document.getElementById("register-form"),
  authMessage: document.getElementById("auth-message"),
  logoutBtn: document.getElementById("logout-btn"),
  pageTitle: document.getElementById("page-title"),
  pageSubtitle: document.getElementById("page-subtitle"),
  licenseForm: document.getElementById("license-form"),
  licensesTableBody: document.getElementById("licenses-table-body"),
  licenseSearch: document.getElementById("license-search"),
  generatedKeys: document.getElementById("generated-keys"),
  generatedKeysOutput: document.getElementById("generated-keys-output"),
  codeExample: document.getElementById("code-example"),
  toast: document.getElementById("toast"),
};

init();

async function init() {
  updateApiEndpointDisplay();
  bindAuthEvents();
  bindDashboardEvents();
  bindTabEvents();
  bindImplementEvents();
  bindLicenseEvents();
  await restoreSession();
}

function updateApiEndpointDisplay() {
  const endpointEl = document.getElementById("cred-api-endpoint");
  const implEndpointEl = document.getElementById("impl-api-url");
  if (endpointEl) endpointEl.textContent = state.apiBaseUrl;
  if (implEndpointEl) implEndpointEl.textContent = state.apiBaseUrl;
}

async function api(path, options = {}) {
  const response = await fetch(`/api${path}`, {
    method: options.method || "GET",
    credentials: "include",
    headers: {
      "Content-Type": "application/json",
      ...(options.headers || {}),
    },
    body: options.body ? JSON.stringify(options.body) : undefined,
  });

  let data = {};
  try {
    data = await response.json();
  } catch {
    data = {};
  }

  if (!response.ok) {
    throw new Error(data.message || "Request failed");
  }

  return data;
}

function applySessionPayload(data) {
  state.currentUser = data.user;
  state.licenses = data.licenses || [];
  state.stats = data.stats || { total: 0, unused: 0, used: 0, banned: 0 };
}

async function restoreSession() {
  try {
    const data = await api("/auth/me");
    applySessionPayload(data);
    showApp();
  } catch {
    showAuth();
  }
}

function bindAuthEvents() {
  document.querySelectorAll("[data-auth-tab]").forEach((tab) => {
    tab.addEventListener("click", () => switchAuthTab(tab.dataset.authTab));
  });

  elements.loginForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    await handleLogin();
  });

  elements.registerForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    await handleRegister();
  });

  elements.logoutBtn.addEventListener("click", handleLogout);
}

function switchAuthTab(tab) {
  document.querySelectorAll(".auth-tab").forEach((button) => {
    button.classList.toggle("active", button.dataset.authTab === tab);
  });
  elements.loginForm.classList.toggle("hidden", tab !== "login");
  elements.registerForm.classList.toggle("hidden", tab !== "register");
  setAuthMessage("");
}

async function handleRegister() {
  const username = document.getElementById("register-username").value.trim();
  const email = document.getElementById("register-email").value.trim();
  const password = document.getElementById("register-password").value;
  const appName = document.getElementById("register-appname").value.trim();

  try {
    setAuthMessage("Creating account...", "");
    const data = await api("/auth/register", {
      method: "POST",
      body: { username, email, password, appName },
    });
    applySessionPayload(data);
    setAuthMessage("Account created successfully.", "success");
    showApp();
  } catch (error) {
    setAuthMessage(error.message, "error");
  }
}

async function handleLogin() {
  const username = document.getElementById("login-username").value.trim();
  const password = document.getElementById("login-password").value;

  try {
    setAuthMessage("Logging in...", "");
    const data = await api("/auth/login", {
      method: "POST",
      body: { username, password },
    });
    applySessionPayload(data);
    showApp();
  } catch (error) {
    setAuthMessage(error.message, "error");
  }
}

async function handleLogout() {
  try {
    await api("/auth/logout", { method: "POST" });
  } catch {
    // Ignore logout errors and clear UI anyway.
  }
  state.currentUser = null;
  state.licenses = [];
  showAuth();
}

function showAuth() {
  elements.authView.classList.remove("hidden");
  elements.appView.classList.add("hidden");
  switchAuthTab("login");
}

function showApp() {
  elements.authView.classList.add("hidden");
  elements.appView.classList.remove("hidden");
  renderDashboard();
  renderImplement();
  renderLicenses();
  switchTab("dashboard");
}

function setAuthMessage(message, type = "") {
  elements.authMessage.textContent = message;
  elements.authMessage.className = `auth-message ${type}`.trim();
}

function bindTabEvents() {
  document.querySelectorAll(".nav-item[data-tab]").forEach((item) => {
    item.addEventListener("click", () => switchTab(item.dataset.tab));
  });
}

function switchTab(tab) {
  document.querySelectorAll(".nav-item[data-tab]").forEach((item) => {
    item.classList.toggle("active", item.dataset.tab === tab);
  });

  document.querySelectorAll(".tab-panel").forEach((panel) => {
    panel.classList.toggle("active", panel.id === `tab-${tab}`);
  });

  const titles = {
    dashboard: ["Dashboard", "Overview of your application credentials"],
    implement: ["Implement", "Integration details and code examples for your app"],
    licenses: ["Licenses", "Generate and manage unlimited license keys"],
  };

  const [title, subtitle] = titles[tab] || titles.dashboard;
  elements.pageTitle.textContent = title;
  elements.pageSubtitle.textContent = subtitle;
}

function bindDashboardEvents() {
  document.getElementById("regenerate-secret-btn").addEventListener("click", async () => {
    try {
      const data = await api("/user/regenerate-secret", { method: "POST" });
      state.currentUser = data.user;
      renderDashboard();
      renderImplement();
      showToast("Application secret regenerated.");
    } catch (error) {
      showToast(error.message);
    }
  });

  document.querySelectorAll(".copy-btn").forEach((button) => {
    button.addEventListener("click", () => {
      const target = document.getElementById(button.dataset.copy);
      if (target) copyText(target.textContent);
    });
  });
}

function renderDashboard() {
  const user = state.currentUser;
  if (!user) return;

  document.getElementById("sidebar-username").textContent = user.username;
  document.getElementById("sidebar-appname").textContent = user.appName;
  document.getElementById("sidebar-avatar").textContent = user.username.charAt(0).toUpperCase();

  document.getElementById("cred-owner-id").textContent = user.ownerId;
  document.getElementById("cred-app-name").textContent = user.appName;
  document.getElementById("cred-app-secret").textContent = user.appSecret;
  document.getElementById("cred-api-endpoint").textContent = state.apiBaseUrl;

  document.getElementById("stat-total-licenses").textContent = state.stats.total;
  document.getElementById("stat-unused-licenses").textContent = state.stats.unused;
  document.getElementById("stat-used-licenses").textContent = state.stats.used;
  document.getElementById("stat-banned-licenses").textContent = state.stats.banned;
}

function bindImplementEvents() {
  document.querySelectorAll(".lang-tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      state.selectedLang = tab.dataset.lang;
      document.querySelectorAll(".lang-tab").forEach((button) => {
        button.classList.toggle("active", button.dataset.lang === state.selectedLang);
      });
      renderImplement();
    });
  });

  document.getElementById("copy-code-btn").addEventListener("click", () => {
    copyText(elements.codeExample.textContent);
  });
}

function renderImplement() {
  const user = state.currentUser;
  if (!user) return;

  document.getElementById("impl-owner-id").textContent = user.ownerId;
  document.getElementById("impl-app-name").textContent = user.appName;
  document.getElementById("impl-app-secret").textContent = user.appSecret;
  updateApiEndpointDisplay();

  elements.codeExample.textContent = getCodeExample(state.selectedLang, user);
}

function getCodeExample(lang, user) {
  const endpoint = state.apiBaseUrl;

  const examples = {
    csharp: `using Astryn;

public static api AstrynApp = new api(
    name: "${user.appName}",
    ownerid: "${user.ownerId}",
    version: "${user.appVersion}",
    path: "${endpoint}"
);

public void InitAstryn()
{
    AstrynApp.init();
    if (!AstrynApp.response.success)
    {
        Console.WriteLine("Status: " + AstrynApp.response.message);
        Environment.Exit(0);
    }
}

public void LoginWithLicense(string key)
{
    AstrynApp.license(key);
    if (AstrynApp.response.success)
        Console.WriteLine("Welcome!");
}`,

    cpp: `#include "Astryn.hpp"

std::string name = "${user.appName}";
std::string ownerid = "${user.ownerId}";
std::string version = "${user.appVersion}";
std::string url = "${endpoint}";

Astryn::api AstrynApp(name, ownerid, version, url);

AstrynApp.init();
if (!AstrynApp.response.success) {
    std::cout << AstrynApp.response.message << std::endl;
    exit(0);
}

AstrynApp.license("LICENSE-KEY-HERE");`,

    python: `from astryn import *

AstrynApp = api(
    name="${user.appName}",
    ownerid="${user.ownerId}",
    version="${user.appVersion}",
    url="${endpoint}",
    hash_to_check=""
)

AstrynApp.init()
if not AstrynApp.response.success:
    print(AstrynApp.response.message)
    os._exit(0)

AstrynApp.license("LICENSE-KEY-HERE")
print(AstrynApp.response.message)`,

    javascript: `const Astryn = require("./astryn");

const AstrynApp = new Astryn({
  name: "${user.appName}",
  ownerid: "${user.ownerId}",
  version: "${user.appVersion}",
  url: "${endpoint}",
});

await AstrynApp.init();
if (!AstrynApp.response.success) {
  console.log(AstrynApp.response.message);
  process.exit(0);
}

await AstrynApp.license("LICENSE-KEY-HERE");
console.log(AstrynApp.response.message);`,
  };

  return examples[lang] || examples.csharp;
}

function bindLicenseEvents() {
  const expiryUnit = document.getElementById("license-expiry-unit");
  const expiryDuration = document.getElementById("license-expiry-duration");

  expiryUnit.addEventListener("change", () => {
    const isLifetime = expiryUnit.value === "lifetime";
    expiryDuration.disabled = isLifetime;
    expiryDuration.closest("label").classList.toggle("hidden", isLifetime);
  });

  elements.licenseForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    await generateLicenses();
  });

  elements.licenseSearch.addEventListener("input", (event) => {
    state.searchQuery = event.target.value.trim().toLowerCase();
    renderLicenses();
  });

  document.getElementById("copy-generated-keys").addEventListener("click", () => {
    copyText(elements.generatedKeysOutput.value);
  });

  document.getElementById("export-licenses-btn").addEventListener("click", exportLicenses);
  document.getElementById("delete-unused-btn").addEventListener("click", deleteUnusedLicenses);
}

async function generateLicenses() {
  const amount = Number(document.getElementById("license-amount").value);
  const mask = document.getElementById("license-mask").value.trim();
  const level = Number(document.getElementById("license-level").value);
  const expiryUnit = document.getElementById("license-expiry-unit").value;
  const expiryDuration = Number(document.getElementById("license-expiry-duration").value);
  const note = document.getElementById("license-note").value.trim();
  const useLower = document.getElementById("license-lowercase").checked;
  const useUpper = document.getElementById("license-uppercase").checked;

  try {
    const data = await api("/licenses", {
      method: "POST",
      body: {
        amount,
        mask,
        level,
        expiryUnit,
        expiryDuration,
        note,
        useLower,
        useUpper,
      },
    });

    state.licenses = data.licenses;
    state.stats = data.stats;
    renderDashboard();
    renderLicenses();

    elements.generatedKeys.classList.remove("hidden");
    elements.generatedKeysOutput.value = (data.keys || []).join("\n");
    showToast(data.message);
  } catch (error) {
    showToast(error.message);
  }
}

function renderLicenses() {
  const licenses = state.licenses.filter((license) => {
    if (!state.searchQuery) return true;
    return (
      license.key.toLowerCase().includes(state.searchQuery) ||
      license.note.toLowerCase().includes(state.searchQuery) ||
      license.status.includes(state.searchQuery)
    );
  });

  elements.licensesTableBody.innerHTML = licenses.length
    ? licenses
        .map(
          (license) => `
      <tr>
        <td><code>${escapeHtml(license.key)}</code></td>
        <td><span class="status-badge status-${license.status}">${license.status}</span></td>
        <td>${license.level}</td>
        <td>${escapeHtml(license.expiryLabel)}</td>
        <td>${escapeHtml(license.note || "-")}</td>
        <td>${formatDate(license.createdAt)}</td>
        <td>
          <div class="table-actions">
            <button type="button" data-action="copy" data-id="${license.id}">Copy</button>
            <button type="button" data-action="toggle-used" data-id="${license.id}">Toggle Used</button>
            <button type="button" data-action="ban" data-id="${license.id}">Ban</button>
            <button type="button" data-action="delete" data-id="${license.id}">Delete</button>
          </div>
        </td>
      </tr>`
        )
        .join("")
    : `<tr><td colspan="7" style="text-align:center;color:var(--text-secondary);">No licenses yet. Generate your first key above.</td></tr>`;

  elements.licensesTableBody.querySelectorAll("button[data-action]").forEach((button) => {
    button.addEventListener("click", () => handleLicenseAction(button.dataset.action, button.dataset.id));
  });
}

async function handleLicenseAction(action, id) {
  const license = state.licenses.find((entry) => entry.id === id);
  if (!license) return;

  if (action === "copy") {
    copyText(license.key);
    return;
  }

  try {
    if (action === "delete") {
      const data = await api(`/licenses/${id}`, { method: "DELETE" });
      state.licenses = data.licenses;
      state.stats = data.stats;
    } else {
      const data = await api(`/licenses/${id}`, {
        method: "PATCH",
        body: { action },
      });
      state.licenses = data.licenses;
      state.stats = data.stats;
    }

    renderDashboard();
    renderLicenses();
    showToast("License updated.");
  } catch (error) {
    showToast(error.message);
  }
}

async function deleteUnusedLicenses() {
  try {
    const data = await api("/licenses", { method: "DELETE" });
    state.licenses = data.licenses;
    state.stats = data.stats;
    renderDashboard();
    renderLicenses();
    showToast(data.message);
  } catch (error) {
    showToast(error.message);
  }
}

function exportLicenses() {
  if (!state.licenses.length) {
    showToast("No licenses to export.");
    return;
  }

  const lines = state.licenses.map(
    (license) =>
      `${license.key}\t${license.status}\t${license.level}\t${license.expiryLabel}\t${license.note || ""}\t${formatDate(license.createdAt)}`
  );

  const blob = new Blob([["Key\tStatus\tLevel\tExpiry\tNote\tCreated", ...lines].join("\n")], {
    type: "text/plain",
  });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = `${state.currentUser.appName.replace(/\s+/g, "_")}_licenses.txt`;
  link.click();
  URL.revokeObjectURL(url);
  showToast("Licenses exported.");
}

function formatDate(timestamp) {
  return new Date(timestamp).toLocaleString();
}

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

function copyText(text) {
  navigator.clipboard.writeText(text).then(
    () => showToast("Copied to clipboard."),
    () => showToast("Could not copy to clipboard.")
  );
}

function showToast(message) {
  elements.toast.textContent = message;
  elements.toast.classList.remove("hidden");
  clearTimeout(showToast.timer);
  showToast.timer = setTimeout(() => elements.toast.classList.add("hidden"), 2400);
}
